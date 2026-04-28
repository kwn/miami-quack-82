#include "get_ready.h"

#include "campaign.h"
#include "font.h"

#include <ace/managers/blit.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/disk_file.h>
#include <ace/utils/extview.h>
#include <ace/utils/font.h>
#include <ace/utils/palette.h>

extern tStateManager *stateMgr;

#define GET_READY_BPP 5
#define GET_READY_COLOR_COUNT (1 << GET_READY_BPP)
#define SCREEN_W 320
#define SCREEN_H 256
#define GET_READY_FULL_LEVEL 15
#define GET_READY_BLACK_LEVEL 0
#define GET_READY_AUTO_ADVANCE_FRAMES 150

#define COLOR_TEXT 29
#define COLOR_SHADOW 23
#define COLOR_BACKGROUND 0

static tView *view;
static tVPort *vport;
static tSimpleBufferManager *buffer;
static tBitMap *image;
static tFont *font;
static tTextBitMap *textBitmap;
static UWORD frameCounter;

#ifdef ACE_USE_AGA_FEATURES
static ULONG pristinePalette[GET_READY_COLOR_COUNT];
#else
static UWORD pristinePalette[GET_READY_COLOR_COUNT];
#endif

static void setPristineColor(UBYTE index, UWORD color) {
#ifdef ACE_USE_AGA_FEATURES
    UBYTE r = (color >> 8) & 0xF;
    UBYTE g = (color >> 4) & 0xF;
    UBYTE b = color & 0xF;
    pristinePalette[index] = ((ULONG)(r * 17) << 16) | ((ULONG)(g * 17) << 8) | (b * 17);
#else
    pristinePalette[index] = color;
#endif
}

static void setPaletteLevel(UBYTE level) {
#ifdef ACE_USE_AGA_FEATURES
    UBYTE agaLevel = ((UWORD)level * 255) / GET_READY_FULL_LEVEL;
    paletteDimAGA(pristinePalette, (ULONG *)vport->pPalette, GET_READY_COLOR_COUNT, agaLevel);
#else
    paletteDim(pristinePalette, vport->pPalette, GET_READY_COLOR_COUNT, level);
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

static UBYTE loadGetReadyImage(void) {
    const LevelDefinition *level = campaignGetCurrentLevel();

    if (!diskFileExists(level->getReadyImagePath) || !diskFileExists(level->getReadyPalettePath)) {
        return 0;
    }

#ifdef ACE_USE_AGA_FEATURES
    paletteLoadFromPath(level->getReadyPalettePath, (UWORD *)pristinePalette, GET_READY_COLOR_COUNT);
#else
    paletteLoadFromPath(level->getReadyPalettePath, pristinePalette, GET_READY_COLOR_COUNT);
#endif
    image = bitmapCreateFromPath(level->getReadyImagePath, 0);
    if (!image) {
        return 0;
    }

    blitCopyAligned(image, 0, 0, buffer->pBack, 0, 0, SCREEN_W, SCREEN_H);
    blitWait();
    return 1;
}

static void drawFallbackScreen(void) {
    const WorldDefinition *world = campaignGetCurrentWorld();
    const LevelDefinition *level = campaignGetCurrentLevel();

    setPristineColor(0, 0x000);
    setPristineColor(COLOR_TEXT, 0xFEA);
    setPristineColor(COLOR_SHADOW, 0x622);

    blitRect(buffer->pBack, 0, 0, SCREEN_W, SCREEN_H, COLOR_BACKGROUND);
    gameFontDrawStr(font, buffer->pBack, textBitmap, 105, 86, "GET READY", COLOR_TEXT, COLOR_SHADOW);
    gameFontDrawStr(font, buffer->pBack, textBitmap, 104, 116, world->name, COLOR_TEXT, COLOR_SHADOW);
    gameFontDrawStr(font, buffer->pBack, textBitmap, 104, 136, level->name, COLOR_TEXT, COLOR_SHADOW);
    gameFontDrawStr(font, buffer->pBack, textBitmap, 72, 190, "FIRE / SPACE TO START", COLOR_TEXT, COLOR_SHADOW);
    blitWait();
}

static void getReadyCreate(void) {
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
        TAG_VPORT_BPP, GET_READY_BPP,
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
        TAG_DONE
    );

    font = fontCreateFromPath("data/fonts/quaver.fnt");
    textBitmap = fontCreateTextBitMap(200, 16);

    if (!loadGetReadyImage()) {
        drawFallbackScreen();
    }

    frameCounter = 0;
    setPaletteLevel(GET_READY_BLACK_LEVEL);
    viewLoad(view);
    fadePaletteLevel(GET_READY_BLACK_LEVEL, GET_READY_FULL_LEVEL);
    systemUnuse();
}

static void getReadyLoop(void) {
    ++frameCounter;

    if (keyUse(KEY_ESCAPE)) {
        fadePaletteLevel(GET_READY_FULL_LEVEL, GET_READY_BLACK_LEVEL);
        campaignSetRoute(CAMPAIGN_ROUTE_TITLE);
        stateChange(stateMgr, &campaignState);
        return;
    }

    if (
        frameCounter >= GET_READY_AUTO_ADVANCE_FRAMES ||
        keyUse(KEY_RETURN) ||
        keyUse(KEY_SPACE) ||
        joyUse(JOY1_FIRE)
    ) {
        fadePaletteLevel(GET_READY_FULL_LEVEL, GET_READY_BLACK_LEVEL);
        campaignSetRoute(CAMPAIGN_ROUTE_GAMEPLAY);
        stateChange(stateMgr, &campaignState);
        return;
    }

    viewProcessManagers(view);
    copProcessBlocks();
    vPortWaitForEnd(vport);
}

static void getReadyDestroy(void) {
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
    if (image) {
        bitmapDestroy(image);
        image = 0;
    }
    if (view) {
        viewDestroy(view);
        view = 0;
    }

    buffer = 0;
    vport = 0;
}

tState getReadyState = {
    .cbCreate = getReadyCreate,
    .cbLoop = getReadyLoop,
    .cbDestroy = getReadyDestroy,
    .cbSuspend = 0,
    .cbResume = 0,
};
