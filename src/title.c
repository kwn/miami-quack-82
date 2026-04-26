#include "title.h"

#include "font.h"
#include "scroller.h"

#include <ace/managers/blit.h>
#include <ace/managers/game.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/extview.h>
#include <ace/utils/font.h>
#include <ace/utils/palette.h>

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
#define MENU_TEXT_SPACING 20
#define MENU_CURSOR_X (MENU_BOX_X + 6)
#define MENU_CURSOR_RESTORE_X MENU_BOX_X
#define MENU_CURSOR_RESTORE_W 16
#define MENU_CURSOR_RESTORE_H 12

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
    MENU_ACTION_START,
    MENU_ACTION_OPTIONS,
    MENU_ACTION_ENTER_CODE,
    MENU_ACTION_EXIT
} MenuAction;

typedef struct {
    const char *text;
    MenuAction action;
} MenuItem;

static const MenuItem menuItems[] = {
    { "START GAME", MENU_ACTION_START },
    { "OPTIONS",    MENU_ACTION_OPTIONS },
    { "ENTER CODE", MENU_ACTION_ENTER_CODE },
    { "EXIT",       MENU_ACTION_EXIT },
};

#define MENU_ITEM_COUNT (sizeof(menuItems) / sizeof(menuItems[0]))

static tView *view;
static tVPort *vport;
static tSimpleBufferManager *buffer;
static tBitMap *titleBitmap;
static tFont *font;
static tTextBitMap *textBitmap;
static UWORD pristinePalette[TITLE_COLOR_COUNT];

static TitlePhase phase;
static UBYTE selectedItem;

static void drawText(tBitMap *dest, UWORD x, UWORD y, const char *text, UBYTE color) {
    gameFontDrawStr(font, dest, textBitmap, x, y, text, color, COLOR_SHADOW);
}

static void presentBackBuffer(void) {
    viewProcessManagers(view);
    copProcessBlocks();
}

static void setPaletteLevel(UBYTE level) {
    paletteDim(pristinePalette, vport->pPalette, TITLE_COLOR_COUNT, level);
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

static void drawSplash(void) {
    blitCopyAligned(titleBitmap, 0, 0, buffer->pBack, 0, 0, SCREEN_W, SCREEN_H);
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

static void drawMenuBase(tBitMap *dest) {
    blitCopyAligned(titleBitmap, 0, 0, dest, 0, 0, SCREEN_W, SCREEN_H);
    blitWait();
    applyCheckerOverlay(dest, MENU_BOX_X, MENU_BOX_Y, MENU_BOX_W, MENU_BOX_H);

    for (UBYTE i = 0; i < MENU_ITEM_COUNT; ++i) {
        UWORD y = MENU_TEXT_Y + (i * MENU_TEXT_SPACING);
        drawText(dest, MENU_TEXT_X, y, menuItems[i].text, COLOR_TEXT);
    }
}

static void restoreCursorSlots(tBitMap *dest) {
    for (UBYTE i = 0; i < MENU_ITEM_COUNT; ++i) {
        UWORD y = MENU_TEXT_Y + (i * MENU_TEXT_SPACING);
        blitCopyAligned(
            titleBitmap, MENU_CURSOR_RESTORE_X, y,
            dest, MENU_CURSOR_RESTORE_X, y,
            MENU_CURSOR_RESTORE_W, MENU_CURSOR_RESTORE_H
        );
        blitWait();
        applyCheckerOverlay(dest, MENU_CURSOR_RESTORE_X, y, MENU_CURSOR_RESTORE_W, MENU_CURSOR_RESTORE_H);
    }
}

static void drawMenuCursor(tBitMap *dest) {
    UWORD y = MENU_TEXT_Y + (selectedItem * MENU_TEXT_SPACING);
    drawText(dest, MENU_CURSOR_X, y, ">", COLOR_SELECTED);
}

static void drawFullMenu(tBitMap *dest) {
    drawMenuBase(dest);
    drawMenuCursor(dest);
}

static void redrawCursor(void) {
    restoreCursorSlots(buffer->pBack);
    drawMenuCursor(buffer->pBack);
}

static void showMenu(void) {
    phase = TITLE_PHASE_MENU;
    selectedItem = 0;

    fadePaletteLevel(PALETTE_FULL_LEVEL, MENU_DIM_LEVEL);

    presentBackBuffer();
    vPortWaitForEnd(vport);

    drawFullMenu(buffer->pBack);
}

static void selectPrevious(void) {
    if (selectedItem == 0) {
        selectedItem = MENU_ITEM_COUNT - 1;
    } else {
        --selectedItem;
    }
}

static void selectNext(void) {
    ++selectedItem;
    if (selectedItem >= MENU_ITEM_COUNT) {
        selectedItem = 0;
    }
}

static void activateSelected(void) {
    switch (menuItems[selectedItem].action) {
        case MENU_ACTION_START:
            fadePaletteLevel(MENU_DIM_LEVEL, PALETTE_BLACK_LEVEL);
            stateChange(stateMgr, &scrollerState);
            break;
        case MENU_ACTION_EXIT:
            fadePaletteLevel(MENU_DIM_LEVEL, PALETTE_BLACK_LEVEL);
            gameExit();
            break;
        case MENU_ACTION_OPTIONS:
        case MENU_ACTION_ENTER_CODE:
        default:
            break;
    }
}

static void titleCreate(void) {
    view = viewCreate(0,
        TAG_VIEW_GLOBAL_PALETTE, 1,
        TAG_VIEW_GLOBAL_BPP, 1,
        TAG_DONE
    );

    vport = vPortCreate(0,
        TAG_VPORT_VIEW, view,
        TAG_VPORT_BPP, TITLE_BPP,
        TAG_VPORT_WIDTH, SCREEN_W,
        TAG_VPORT_HEIGHT, SCREEN_H,
        TAG_DONE
    );

    buffer = simpleBufferCreate(0,
        TAG_SIMPLEBUFFER_VPORT, vport,
        TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
        TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
        TAG_DONE
    );

    paletteLoadFromPath("data/title/title.plt", pristinePalette, TITLE_COLOR_COUNT);
    titleBitmap = bitmapCreateFromPath("data/title/title.bm", 0);
    font = fontCreateFromPath("data/fonts/quaver.fnt");
    textBitmap = fontCreateTextBitMap(160, 16);
    setPaletteLevel(PALETTE_BLACK_LEVEL);

    phase = TITLE_PHASE_SPLASH;
    selectedItem = 0;
    drawSplash();
    presentBackBuffer();

    systemUnuse();
    viewLoad(view);
    fadePaletteLevel(PALETTE_BLACK_LEVEL, PALETTE_FULL_LEVEL);
    drawFullMenu(buffer->pBack);
}

static void titleLoop(void) {
    UBYTE selectionChanged = 0;

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

    if (keyUse(KEY_UP) || joyUse(JOY1_UP)) {
        selectPrevious();
        selectionChanged = 1;
    } else if (keyUse(KEY_DOWN) || joyUse(JOY1_DOWN)) {
        selectNext();
        selectionChanged = 1;
    }

    if (keyUse(KEY_RETURN) || keyUse(KEY_SPACE) || joyUse(JOY1_FIRE)) {
        activateSelected();
        return;
    }

    if (keyUse(KEY_ESCAPE)) {
        gameExit();
        return;
    }

    if (selectionChanged) {
        redrawCursor();
        presentBackBuffer();
    }

    vPortWaitForEnd(vport);
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
