#include "title.h"

#include "campaign.h"
#include "font.h"

#include <ace/managers/blit.h>
#include <ace/managers/game.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/sprite.h>
#include <ace/managers/state.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/extview.h>
#include <ace/utils/font.h>
#include <ace/utils/palette.h>
#include <hardware/dmabits.h>

extern tStateManager *stateMgr;

#define TITLE_BPP 5
#define TITLE_COLOR_COUNT (1 << TITLE_BPP)
#define SCREEN_W 320
#define SCREEN_H 256

#define MENU_BOX_X 160
#define MENU_BOX_Y 120
#define MENU_BOX_W 144
#define MENU_BOX_H 100
#define MENU_TEXT_X (MENU_BOX_X + 16)
#define MENU_TEXT_Y (MENU_BOX_Y + 15)
#define MENU_TEXT_LOCAL_X (MENU_TEXT_X - MENU_BOX_X)
#define MENU_TEXT_LOCAL_Y (MENU_TEXT_Y - MENU_BOX_Y)
#define MENU_TEXT_SPACING 20
#define MENU_CURSOR_X (MENU_BOX_X + 2)
#define MENU_CURSOR_Y_OFFSET -1
#define MENU_CURSOR_H 8
#define MENU_CURSOR_BITMAP_H (MENU_CURSOR_H + 2)
#define CURSOR_ANIM_FRAMES (sizeof(cursorAnimOffsets) / sizeof(cursorAnimOffsets[0]))

#define COLOR_TEXT 29
#define COLOR_SELECTED 12
#define COLOR_SHADOW 23
#define MENU_DIM_LEVEL 10
#define PALETTE_FULL_LEVEL 15
#define PALETTE_BLACK_LEVEL 0

typedef enum {
    TITLE_PHASE_SPLASH,
    TITLE_PHASE_MENU
} TitlePhase;

typedef enum {
    MENU_ACTION_NONE,
    MENU_ACTION_START,
    MENU_ACTION_OPTIONS,
    MENU_ACTION_MAIN,
    MENU_ACTION_CODE,
    MENU_ACTION_EXIT
} MenuAction;

typedef enum {
    MENU_ITEM_ACTION,
    MENU_ITEM_OPTION
} MenuItemKind;

typedef enum {
    MENU_PAGE_MAIN,
    MENU_PAGE_OPTIONS
} MenuPageId;

typedef struct {
    const char **labels;
    UBYTE labelCount;
    UBYTE *value;
} MenuOption;

typedef struct {
    MenuItemKind kind;
    const char *text;
    MenuAction action;
    MenuOption option;
} MenuItem;

typedef struct {
    const MenuItem *items;
    UBYTE itemCount;
} MenuPage;

static UBYTE optDifficulty = 1;
static UBYTE optControl = 0;
static UBYTE optGore = 0;

static const char *difficultyLabels[] = {
    "DIFF: EASY",
    "DIFF: NORMAL",
    "DIFF: HARD",
    "DIFF: INSANE",
};

static const char *controlLabels[] = {
    "CTRL: KBD+MOUSE",
    "CTRL: JOYSTICK",
    "CTRL: JOY+AUTO",
};

static const char *goreLabels[] = {
    "GORE: YES",
    "GORE: NO",
};

static const MenuItem mainMenuItems[] = {
    { MENU_ITEM_ACTION, "START GAME", MENU_ACTION_START,   { 0, 0, 0 } },
    { MENU_ITEM_ACTION, "OPTIONS",    MENU_ACTION_OPTIONS, { 0, 0, 0 } },
    { MENU_ITEM_ACTION, "ENTER CODE", MENU_ACTION_CODE,    { 0, 0, 0 } },
    { MENU_ITEM_ACTION, "EXIT",       MENU_ACTION_EXIT,    { 0, 0, 0 } },
};

static const MenuItem optionsMenuItems[] = {
    { MENU_ITEM_OPTION, 0,      MENU_ACTION_NONE, { difficultyLabels, 4, &optDifficulty } },
    { MENU_ITEM_OPTION, 0,      MENU_ACTION_NONE, { controlLabels,    3, &optControl    } },
    { MENU_ITEM_OPTION, 0,      MENU_ACTION_NONE, { goreLabels,       2, &optGore       } },
    { MENU_ITEM_ACTION, "BACK", MENU_ACTION_MAIN, { 0, 0, 0 } },
};

static const MenuPage menuPages[] = {
    { mainMenuItems,    sizeof(mainMenuItems) / sizeof(mainMenuItems[0]) },
    { optionsMenuItems, sizeof(optionsMenuItems) / sizeof(optionsMenuItems[0]) },
};

static const UBYTE cursorAnimOffsets[] = {
    0, 0, 1, 2, 3, 4, 5, 5, 6, 6, 6, 5, 5, 4, 3, 2, 1, 0
};

static const UWORD menuCursorData[MENU_CURSOR_H][2] = {
    { 0x8000, 0x8000 },
    { 0xC000, 0xC000 },
    { 0xE000, 0xE000 },
    { 0xF000, 0xF000 },
    { 0xE000, 0xE000 },
    { 0xC000, 0xC000 },
    { 0x8000, 0x8000 },
    { 0x0000, 0x0000 },
};

static tView *view;
static tVPort *vport;
static tSimpleBufferManager *buffer;
static tBitMap *titleBitmap;
static tBitMap *menuBaseBitmap;
static tBitMap *menuDrawBitmap;
static tBitMap *cursorBitmap;
static tSprite *cursorSprite;
static tFont *font;
static tTextBitMap *textBitmap;
#ifdef ACE_USE_AGA_FEATURES
static ULONG pristinePalette[TITLE_COLOR_COUNT];
#else
static UWORD pristinePalette[TITLE_COLOR_COUNT];
#endif

static TitlePhase phase;
static MenuPageId currentPage;
static UBYTE selectedItem;
static UBYTE cursorAnimFrame;
static UBYTE cursorAnimTick;
static UBYTE pendingRowRefresh;
static UBYTE pendingRow;

static void drawText(tBitMap *dest, UWORD x, UWORD y, const char *text, UBYTE color) {
    gameFontDrawStr(font, dest, textBitmap, x, y, text, color, COLOR_SHADOW);
}

static const MenuPage *getCurrentMenuPage(void) {
    return &menuPages[currentPage];
}

static const MenuItem *getSelectedMenuItem(void) {
    const MenuPage *page = getCurrentMenuPage();
    return &page->items[selectedItem];
}

static const char *getMenuItemText(const MenuItem *item) {
    if (item->kind == MENU_ITEM_OPTION) {
        return item->option.labels[*item->option.value];
    }
    return item->text;
}

static void presentBackBuffer(void) {
    spriteProcessChannel(0);
    viewProcessManagers(view);
    copProcessBlocks();
}

static void setPaletteLevel(UBYTE level) {
#ifdef ACE_USE_AGA_FEATURES
    UBYTE agaLevel = ((UWORD)level * 255) / PALETTE_FULL_LEVEL;
    paletteDimAga(pristinePalette, (ULONG *)vport->pPalette, TITLE_COLOR_COUNT, agaLevel);
#else
    paletteDim(pristinePalette, vport->pPalette, TITLE_COLOR_COUNT, level);
#endif
    viewUpdateGlobalPalette(view);
}

static void fadePaletteLevel(BYTE fromLevel, BYTE toLevel) {
    BYTE step = (fromLevel < toLevel) ? 1 : -1;

    for (BYTE level = fromLevel; ; level += step) {
        setPaletteLevel((UBYTE)level);
        vPortWaitForEnd(vport);

        if (level == toLevel) {
            break;
        }
    }
}

static void buildCursorSpriteBitmap(void) {
    cursorBitmap = bitmapCreate(16, MENU_CURSOR_BITMAP_H, 2, BMF_CLEAR | BMF_INTERLEAVED);

    for (UBYTE row = 0; row < MENU_CURSOR_H; ++row) {
        UWORD *plane0 = (UWORD *)(cursorBitmap->Planes[0] + ((row + 1) * cursorBitmap->BytesPerRow));
        UWORD *plane1 = (UWORD *)(cursorBitmap->Planes[1] + ((row + 1) * cursorBitmap->BytesPerRow));
        *plane0 = menuCursorData[row][0];
        *plane1 = menuCursorData[row][1];
    }
}

static void updateCursorSprite(void) {
    WORD y = MENU_TEXT_Y + (selectedItem * MENU_TEXT_SPACING) + MENU_CURSOR_Y_OFFSET;

    cursorSprite->wX = MENU_CURSOR_X + cursorAnimOffsets[cursorAnimFrame];
    cursorSprite->wY = y;
    spriteRequestMetadataUpdate(cursorSprite);
    spriteProcess(cursorSprite);
}

static void animateCursorSprite(void) {
    cursorAnimTick = (cursorAnimTick + 1) & 3;
    if (cursorAnimTick != 0) {
        ++cursorAnimFrame;
        if (cursorAnimFrame >= CURSOR_ANIM_FRAMES) {
            cursorAnimFrame = 0;
        }
        updateCursorSprite();
    }
}

static void drawSplash(tBitMap *dest) {
    blitCopyAligned(titleBitmap, 0, 0, dest, 0, 0, SCREEN_W, SCREEN_H);
    blitWait();
}

static void applyCheckerOverlay(tBitMap *bitmap, UWORD x, UWORD y, UWORD width, UWORD height) {
    UWORD byteX = x >> 3;
    UWORD byteWidth = width >> 3;

    for (UBYTE plane = 0; plane < bitmap->Depth; ++plane) {
        for (UWORD row = 0; row < height; ++row) {
            UBYTE *dst = bitmap->Planes[plane] + ((y + row) * bitmap->BytesPerRow) + byteX;
            UBYTE mask = ((y + row) & 1) ? 0x55 : 0xAA;

            for (UWORD byte = 0; byte < byteWidth; ++byte) {
                dst[byte] &= ~mask;
            }
        }
    }
}

static void buildMenuBaseBitmap(void) {
    blitCopyAligned(
        titleBitmap, MENU_BOX_X, MENU_BOX_Y,
        menuBaseBitmap, 0, 0,
        MENU_BOX_W, MENU_BOX_H
    );
    blitWait();
    applyCheckerOverlay(menuBaseBitmap, 0, 0, MENU_BOX_W, MENU_BOX_H);
}

static void copyMenuDrawToBackBuffer(UWORD srcY, UWORD dstY, UWORD height) {
    blitCopyAligned(
        menuDrawBitmap, 0, srcY,
        buffer->pBack, MENU_BOX_X, dstY,
        MENU_BOX_W, height
    );
    blitWait();
}

static void drawMenuPageToBuffer(void) {
    const MenuPage *page = getCurrentMenuPage();

    blitCopyAligned(menuBaseBitmap, 0, 0, menuDrawBitmap, 0, 0, MENU_BOX_W, MENU_BOX_H);
    blitWait();

    for (UBYTE i = 0; i < page->itemCount; ++i) {
        UWORD y = MENU_TEXT_LOCAL_Y + (i * MENU_TEXT_SPACING);
        drawText(menuDrawBitmap, MENU_TEXT_LOCAL_X, y, getMenuItemText(&page->items[i]), COLOR_TEXT);
    }
}

static void drawMenuToBackBuffer(void) {
    drawMenuPageToBuffer();
    copyMenuDrawToBackBuffer(0, MENU_BOX_Y, MENU_BOX_H);
}

static void drawMenuRowToBackBuffer(UBYTE row) {
    const MenuPage *page = getCurrentMenuPage();
    UWORD srcY = MENU_TEXT_LOCAL_Y + (row * MENU_TEXT_SPACING);
    UWORD dstY = MENU_TEXT_Y + (row * MENU_TEXT_SPACING);

    blitCopyAligned(
        menuBaseBitmap, 0, srcY,
        menuDrawBitmap, 0, srcY,
        MENU_BOX_W, MENU_TEXT_SPACING
    );
    blitWait();
    drawText(menuDrawBitmap, MENU_TEXT_LOCAL_X, srcY, getMenuItemText(&page->items[row]), COLOR_TEXT);
    copyMenuDrawToBackBuffer(srcY, dstY, MENU_TEXT_SPACING);
}

static void refreshMenuPage(void) {
    pendingRowRefresh = 0;
    drawMenuToBackBuffer();
    updateCursorSprite();
    presentBackBuffer();
    vPortWaitForEnd(vport);
    drawMenuToBackBuffer();
}

static void refreshMenuRow(UBYTE row) {
    drawMenuRowToBackBuffer(row);
    updateCursorSprite();
    presentBackBuffer();
    pendingRow = row;
    pendingRowRefresh = 1;
}

static void processPendingRowRefresh(void) {
    if (!pendingRowRefresh) {
        return;
    }

    drawMenuRowToBackBuffer(pendingRow);
    pendingRowRefresh = 0;
}

static void showMenu(void) {
    phase = TITLE_PHASE_MENU;
    currentPage = MENU_PAGE_MAIN;
    selectedItem = 0;
    cursorAnimFrame = 0;
    cursorAnimTick = 0;

    fadePaletteLevel(PALETTE_FULL_LEVEL, MENU_DIM_LEVEL);

    spriteSetEnabled(cursorSprite, 1);
    updateCursorSprite();
    presentBackBuffer();
    vPortWaitForEnd(vport);

    drawMenuToBackBuffer();
}

static void moveSelection(BYTE delta) {
    const MenuPage *page = getCurrentMenuPage();

    if (delta < 0) {
        selectedItem = (selectedItem == 0) ? page->itemCount - 1 : selectedItem - 1;
    } else {
        ++selectedItem;
        if (selectedItem >= page->itemCount) {
            selectedItem = 0;
        }
    }
}

static void changeOption(const MenuItem *item, BYTE delta) {
    UBYTE value = *item->option.value;

    if (delta < 0) {
        value = (value == 0) ? item->option.labelCount - 1 : value - 1;
    } else {
        ++value;
        if (value >= item->option.labelCount) {
            value = 0;
        }
    }

    *item->option.value = value;
    refreshMenuRow(selectedItem);
}

static void showPage(MenuPageId page, UBYTE item) {
    currentPage = page;
    selectedItem = item;
    refreshMenuPage();
}

static void activateSelected(void) {
    const MenuItem *item = getSelectedMenuItem();

    if (item->kind == MENU_ITEM_OPTION) {
        changeOption(item, 1);
        return;
    }

    switch (item->action) {
        case MENU_ACTION_START:
            fadePaletteLevel(MENU_DIM_LEVEL, PALETTE_BLACK_LEVEL);
            campaignStartNewGame();
            stateChange(stateMgr, &campaignState);
            break;
        case MENU_ACTION_OPTIONS:
            showPage(MENU_PAGE_OPTIONS, 0);
            break;
        case MENU_ACTION_MAIN:
            showPage(MENU_PAGE_MAIN, 1);
            break;
        case MENU_ACTION_EXIT:
            fadePaletteLevel(MENU_DIM_LEVEL, PALETTE_BLACK_LEVEL);
            gameExit();
            break;
        case MENU_ACTION_CODE:
        case MENU_ACTION_NONE:
        default:
            break;
    }
}

static void titleCreate(void) {
    view = viewCreate(0,
        TAG_VIEW_GLOBAL_PALETTE, 1,
        TAG_VIEW_GLOBAL_BPP, 1,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VIEW_USES_AGA, 1,
#endif
        TAG_DONE
    );

    vport = vPortCreate(0,
        TAG_VPORT_VIEW, view,
        TAG_VPORT_BPP, TITLE_BPP,
        TAG_VPORT_WIDTH, SCREEN_W,
        TAG_VPORT_HEIGHT, SCREEN_H,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VPORT_USES_AGA, 1,
        TAG_VPORT_FMODE, 0,
#endif
        TAG_DONE
    );

    buffer = simpleBufferCreate(0,
        TAG_SIMPLEBUFFER_VPORT, vport,
        TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
        TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
        TAG_DONE
    );

#ifdef ACE_USE_AGA_FEATURES
    paletteLoadFromPath("data/palettes/title.plt", (UWORD *)pristinePalette, TITLE_COLOR_COUNT);
#else
    paletteLoadFromPath("data/palettes/title.plt", pristinePalette, TITLE_COLOR_COUNT);
#endif
    titleBitmap = bitmapCreateFromPath("data/title/title.bm", 0);
    menuBaseBitmap = bitmapCreate(MENU_BOX_W, MENU_BOX_H, TITLE_BPP, BMF_CLEAR);
    menuDrawBitmap = bitmapCreate(MENU_BOX_W, MENU_BOX_H, TITLE_BPP, BMF_CLEAR);
    buildMenuBaseBitmap();
    font = fontCreateFromPath("data/fonts/quaver.fnt");
    textBitmap = fontCreateTextBitMap(160, 16);
    setPaletteLevel(PALETTE_BLACK_LEVEL);
    spriteManagerCreate(view, 0, 0);
    buildCursorSpriteBitmap();
    cursorSprite = spriteAdd(0, cursorBitmap);
    spriteSetEnabled(cursorSprite, 0);
    systemSetDmaBit(DMAB_SPRITE, 1);

    phase = TITLE_PHASE_SPLASH;
    currentPage = MENU_PAGE_MAIN;
    selectedItem = 0;
    drawSplash(buffer->pBack);
    presentBackBuffer();

    systemUnuse();
    viewLoad(view);
    fadePaletteLevel(PALETTE_BLACK_LEVEL, PALETTE_FULL_LEVEL);
    drawSplash(buffer->pBack);
    drawMenuToBackBuffer();
}

static void titleLoop(void) {
    UBYTE selectionChanged = 0;
    const MenuItem *selectedMenuItem;

    if (phase == TITLE_PHASE_SPLASH) {
        if (keyUse(KEY_RETURN) || keyUse(KEY_SPACE) || joyUse(JOY1_FIRE)) {
            showMenu();
            return;
        }
        if (keyUse(KEY_ESCAPE)) {
            gameExit();
            return;
        }

        vPortWaitForEnd(vport);
        return;
    }

    selectedMenuItem = getSelectedMenuItem();

    if (keyUse(KEY_UP) || joyUse(JOY1_UP)) {
        moveSelection(-1);
        selectionChanged = 1;
    } else if (keyUse(KEY_DOWN) || joyUse(JOY1_DOWN)) {
        moveSelection(1);
        selectionChanged = 1;
    } else if ((keyUse(KEY_LEFT) || joyUse(JOY1_LEFT)) && selectedMenuItem->kind == MENU_ITEM_OPTION) {
        changeOption(selectedMenuItem, -1);
        return;
    } else if ((keyUse(KEY_RIGHT) || joyUse(JOY1_RIGHT)) && selectedMenuItem->kind == MENU_ITEM_OPTION) {
        changeOption(selectedMenuItem, 1);
        return;
    }

    if (keyUse(KEY_RETURN) || keyUse(KEY_SPACE) || joyUse(JOY1_FIRE)) {
        activateSelected();
        return;
    }

    if (keyUse(KEY_ESCAPE)) {
        if (currentPage == MENU_PAGE_OPTIONS) {
            showPage(MENU_PAGE_MAIN, 1);
        } else {
            gameExit();
        }
        return;
    }

    if (selectionChanged) {
        updateCursorSprite();
    }

    animateCursorSprite();
    spriteProcessChannel(0);
    copProcessBlocks();
    vPortWaitForEnd(vport);
    processPendingRowRefresh();
}

static void titleDestroy(void) {
    systemUse();
    viewLoad(0);

    if (textBitmap) {
        fontDestroyTextBitMap(textBitmap);
        textBitmap = 0;
    }
    if (font) {
        fontDestroy(font);
        font = 0;
    }
    if (titleBitmap) {
        bitmapDestroy(titleBitmap);
        titleBitmap = 0;
    }
    if (menuDrawBitmap) {
        bitmapDestroy(menuDrawBitmap);
        menuDrawBitmap = 0;
    }
    if (menuBaseBitmap) {
        bitmapDestroy(menuBaseBitmap);
        menuBaseBitmap = 0;
    }
    if (cursorSprite) {
        spriteRemove(cursorSprite);
        cursorSprite = 0;
    }
    systemSetDmaBit(DMAB_SPRITE, 0);
    spriteManagerDestroy();
    if (cursorBitmap) {
        bitmapDestroy(cursorBitmap);
        cursorBitmap = 0;
    }
    if (view) {
        viewDestroy(view);
        view = 0;
    }

    buffer = 0;
    vport = 0;
}

tState titleState = {
    .cbCreate  = titleCreate,
    .cbLoop    = titleLoop,
    .cbDestroy = titleDestroy,
    .cbSuspend = 0,
    .cbResume  = 0,
};
