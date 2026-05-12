#include "get_ready.h"

#include "campaign.h"
#include "font.h"
#include "game.h"
#include "map_generator.h"

#include <ace/managers/blit.h>
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

#define GET_READY_COLOR_COUNT GAME_COLOR_COUNT
#define SCREEN_W 320
#define SCREEN_H 256
#define GET_READY_FULL_LEVEL 15
#define GET_READY_BLACK_LEVEL 0
#define GET_READY_AUTO_ADVANCE_FRAMES 150

static tView *view;
static tVPort *vport;
static tSimpleBufferManager *buffer;
static tBitMap *image;
static tFont *font;
static tTextBitMap *textBitmap;
static UWORD frameCounter;
static UBYTE isFadedOut;

#ifdef ACE_USE_AGA_FEATURES
static ULONG pristinePalette[GET_READY_COLOR_COUNT];
#else
static UWORD pristinePalette[GET_READY_COLOR_COUNT];
#endif

static void setPaletteLevel(UBYTE level) {
#ifdef ACE_USE_AGA_FEATURES
    UBYTE agaLevel = ((UWORD)level * 255) / GET_READY_FULL_LEVEL;
    paletteDimAga(pristinePalette, (ULONG *)vport->pPalette, GET_READY_COLOR_COUNT, agaLevel);
#else
    paletteDimOcs(pristinePalette, vport->pPalette, GET_READY_COLOR_COUNT, level);
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

static void loadGetReadyImage(void) {
    const LevelDefinition *level = campaignGetCurrentLevel();

#ifdef ACE_USE_AGA_FEATURES
    paletteLoadFromPath(level->getReadyPalettePath, (UWORD *)pristinePalette, GET_READY_COLOR_COUNT);
#else
    paletteLoadFromPath(level->getReadyPalettePath, pristinePalette, GET_READY_COLOR_COUNT);
#endif
    image = bitmapCreateFromPath(level->getReadyImagePath, 0);
    blitCopyAligned(image, 0, 0, buffer->pBack, 0, 0, SCREEN_W, SCREEN_H);
    blitWait();
}

static void generateCurrentMap(void) {
    const WorldDefinition *world = campaignGetCurrentWorld();

    switch (world->kind) {
        case WORLD_KIND_LAKE:
            mapGeneratorGenerateLake();
            break;
        case WORLD_KIND_OUTDOOR:
        default:
            mapGeneratorGenerateOutdoor();
            break;
    }
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
        TAG_VPORT_BPP, GAME_BPP,
        TAG_VPORT_WIDTH, SCREEN_W,
        TAG_VPORT_HEIGHT, SCREEN_H,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VPORT_USES_AGA, 1,
        TAG_VPORT_FMODE, 3,
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

    loadGetReadyImage();

    frameCounter = 0;
    isFadedOut = 0;
    setPaletteLevel(GET_READY_BLACK_LEVEL);
    viewLoad(view);
    fadePaletteLevel(GET_READY_BLACK_LEVEL, GET_READY_FULL_LEVEL);
    mapGeneratorSetSeed(0x19820827);
    generateCurrentMap();
    fadePaletteLevel(GET_READY_FULL_LEVEL, GET_READY_BLACK_LEVEL);
    isFadedOut = 1;
    gamePrepare();
    frameCounter = GET_READY_AUTO_ADVANCE_FRAMES;
    systemUnuse();
}

static void getReadyLoop(void) {
    ++frameCounter;

    if (keyUse(KEY_ESCAPE)) {
        systemUse();
        gameDiscardPrepared();
        systemUnuse();
        if (!isFadedOut) {
            fadePaletteLevel(GET_READY_FULL_LEVEL, GET_READY_BLACK_LEVEL);
            isFadedOut = 1;
        }
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
        if (!isFadedOut) {
            fadePaletteLevel(GET_READY_FULL_LEVEL, GET_READY_BLACK_LEVEL);
            isFadedOut = 1;
        }
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
    isFadedOut = 0;
}

tState getReadyState = {
    .cbCreate = getReadyCreate,
    .cbLoop = getReadyLoop,
    .cbDestroy = getReadyDestroy,
    .cbSuspend = 0,
    .cbResume = 0,
};
