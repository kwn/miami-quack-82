/**
 * scroller.c – XY tile-buffer scrolling demo
 *
 * Map  : 128 × 128 tiles, 16 px each  →  2048 × 2048 pixel world
 * View : 320 × 256 px (PAL lores), 4 bpp (16 colours)
 * Input: arrow keys move camera, ESC exits
 *
 * Tileset is built at runtime with blitRect – no asset files needed.
 * Five distinct tile types make every map position visually unique so
 * the scrolling is immediately obvious.
 */

#include "scroller.h"

#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/utils/extview.h>

/* ------------------------------------------------------------------ sizes */

#define SCREEN_W     320
#define SCREEN_H     256
#define BPP            4   /* 16 colours */

#define TILE_SHIFT     4   /* 1 << 4 = 16 px per tile */
#define TILE_SIZE     16

#define MAP_TILES_X  128
#define MAP_TILES_Y  128

#define CAM_SPEED      4   /* px per frame */

/* ------------------------------------------------------------------ tiles */

#define TILE_GRASS   0   /* green meadow            */
#define TILE_DIRT    1   /* brown earth             */
#define TILE_WATER   2   /* blue lake               */
#define TILE_STONE   3   /* grey rock / wall        */
#define TILE_SAND    4   /* yellow beach            */
#define TILE_COUNT   5

/* ---------------------------------------------------------------- palette
 * 12-bit Amiga OCS colour: 0xRGB
 * Indices 0..4  : tile base colours (one dominant colour per tile)
 * Indices 5..9  : tile highlight / detail colours
 * Indices 10..15: spare / future use
 */
static const UWORD s_pPalette[1 << BPP] = {
    /* 0  grass dark  */ 0x0271,
    /* 1  dirt  dark  */ 0x0531,
    /* 2  water dark  */ 0x0027,
    /* 3  stone dark  */ 0x0445,
    /* 4  sand  dark  */ 0x0AA4,
    /* 5  grass light */ 0x04C2,
    /* 6  dirt  light */ 0x0A62,
    /* 7  water light */ 0x024B,
    /* 8  stone light */ 0x0778,
    /* 9  sand  light */ 0x0EC7,
    /* 10 black       */ 0x0000,
    /* 11 dark grey   */ 0x0333,
    /* 12 mid grey    */ 0x0666,
    /* 13 light grey  */ 0x0999,
    /* 14 white       */ 0x0FFF,
    /* 15 accent red  */ 0x0F22,
};

/* ------------------------------------------------------------------ state */

static tView              *s_pView;
static tVPort             *s_pVPort;
static tTileBufferManager *s_pTileBuf;
static tBitMap            *s_pTileSet;

/* --------------------------------------------------------- tileset builder
 *
 * Every tile is TILE_SIZE × TILE_SIZE px and lives at y = index * TILE_SIZE
 * inside the single-column tileset bitmap.
 *
 * Each tile uses two palette colours (dark base + light highlight) so the
 * tiles are distinguishable at a glance.  The highlight is a 2-px stripe or
 * checkerboard-style band – everything done with blitRect.
 */
static void buildTileSet(void) {
    s_pTileSet = bitmapCreate(
        TILE_SIZE,
        TILE_SIZE * TILE_COUNT,
        BPP,
        BMF_CLEAR | BMF_INTERLEAVED
    );

    for (UBYTE t = 0; t < TILE_COUNT; ++t) {
        UWORD uwBase = TILE_SIZE * t;

        /* fill entire tile with dark base colour (index = t) */
        blitRect(s_pTileSet, 0, uwBase, TILE_SIZE, TILE_SIZE, t);

        /* add a light 2-px horizontal band 4 px from the top */
        blitRect(s_pTileSet, 0, uwBase + 4, TILE_SIZE, 2, t + 5);

        /* add a 2-px vertical stripe 4 px from the left – creates a
         * simple "brick" cross-hatch that makes tile edges visible */
        blitRect(s_pTileSet, 4, uwBase, 2, TILE_SIZE, t + 5);
    }
}

/* ----------------------------------------------------------- map generator
 *
 * Pure deterministic noise: combines tile X and Y coordinates with a few
 * prime multipliers so every position looks different without needing
 * a real noise library or random seed.
 *
 * The result is a varied landscape:
 *   - mostly grass, patches of dirt and sand
 *   - occasional water pools and stone outcrops
 */
static UBYTE tileAt(UWORD uwX, UWORD uwY) {
    UWORD v = (uwX * 7u + uwY * 13u + (uwX ^ uwY) * 3u) & 0xFFu;
    if      (v < 80)  return TILE_GRASS;
    else if (v < 130) return TILE_DIRT;
    else if (v < 170) return TILE_SAND;
    else if (v < 210) return TILE_STONE;
    else              return TILE_WATER;
}

static void buildMap(void) {
    for (UWORD x = 0; x < MAP_TILES_X; ++x) {
        for (UWORD y = 0; y < MAP_TILES_Y; ++y) {
            s_pTileBuf->pTileData[x][y] = tileAt(x, y);
        }
    }
}

/* ---------------------------------------------------- ACE state callbacks */

static void scrollerCreate(void) {
    s_pView = viewCreate(0, TAG_DONE);

    s_pVPort = vPortCreate(0,
        TAG_VPORT_VIEW, s_pView,
        TAG_VPORT_BPP,  BPP,
        TAG_DONE
    );

    for (UBYTE i = 0; i < (1 << BPP); ++i) {
        s_pVPort->pPalette[i] = s_pPalette[i];
    }

    buildTileSet();

    s_pTileBuf = tileBufferCreate(0,
        TAG_TILEBUFFER_VPORT,               s_pVPort,
        TAG_TILEBUFFER_BITMAP_FLAGS,        BMF_CLEAR | BMF_INTERLEAVED,
        TAG_TILEBUFFER_BOUND_TILE_X,        MAP_TILES_X,
        TAG_TILEBUFFER_BOUND_TILE_Y,        MAP_TILES_Y,
        TAG_TILEBUFFER_TILE_SHIFT,          TILE_SHIFT,
        TAG_TILEBUFFER_TILESET,             s_pTileSet,
        TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
        TAG_DONE
    );

    buildMap();
    cameraSetCoord(s_pTileBuf->pCamera, 0, 0);
    tileBufferRedrawAll(s_pTileBuf);

    viewLoad(s_pView);
    systemUnuse();
}

static void scrollerLoop(void) {
    /* camera movement */
    WORD wDx = 0, wDy = 0;
    if (keyCheck(KEY_LEFT))  { wDx = -CAM_SPEED; }
    if (keyCheck(KEY_RIGHT)) { wDx =  CAM_SPEED;  }
    if (keyCheck(KEY_UP))    { wDy = -CAM_SPEED; }
    if (keyCheck(KEY_DOWN))  { wDy =  CAM_SPEED;  }

    if (keyUse(KEY_ESCAPE)) {
        gameExit();
        return;
    }

    cameraMoveBy(s_pTileBuf->pCamera, wDx, wDy);
    tileBufferProcess(s_pTileBuf);
    viewProcessManagers(s_pView);
    copProcessBlocks();
    vPortWaitForEnd(s_pVPort);
}

static void scrollerDestroy(void) {
    systemUse();
    viewLoad(0);
    viewDestroy(s_pView);     /* also frees vPort, tileBuf, scrollBuf, camera */
    bitmapDestroy(s_pTileSet);
}

/* -------------------------------------------------- exported state struct */

tState g_sScrollerState = {
    .cbCreate  = scrollerCreate,
    .cbLoop    = scrollerLoop,
    .cbDestroy = scrollerDestroy,
    .cbSuspend = 0,
    .cbResume  = 0,
};
