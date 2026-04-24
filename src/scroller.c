/**
 * scroller.c – XY tile-buffer scrolling demo
 *
 * Map  : 128 × 128 tiles, 16 px each  →  2048 × 2048 pixel world
 * View : 320 × 256 px (PAL lores), 4 bpp (16 colours)
 * Input: arrow keys / WASD move camera, ESC exits
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

#define BPP            4   /* 16 colours */

#define TILE_SHIFT     4   /* 1 << 4 = 16 px per tile */
#define TILE_SIZE     16

#define MAP_TILES_X  128
#define MAP_TILES_Y  128

#define CAM_SPEED      4   /* px per frame */

/* ------------------------------------------------------------------ tiles */

#define TILE_GRASS   0
#define TILE_DIRT    1
#define TILE_WATER   2
#define TILE_STONE   3
#define TILE_SAND    4
#define TILE_COUNT   5

/* ---------------------------------------------------------------- palette
 * 12-bit Amiga OCS colour: 0xRGB
 * Indices 0..4  : tile base colours
 * Indices 5..9  : tile highlight / detail colours
 * Indices 10..15: spare
 */
static const UWORD palette[1 << BPP] = {
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
    /* 10..15 spare   */
    0x0000, 0x0333, 0x0666, 0x0999, 0x0FFF, 0x0F22,
};

/* ------------------------------------------------------------------ state */

static tView              *view;
static tVPort             *vport;
static tTileBufferManager *tileBuf;
static tBitMap            *tileSet;

/* --------------------------------------------------------- tileset builder */

static void buildTileSet(void) {
    tileSet = bitmapCreate(
        TILE_SIZE,
        TILE_SIZE * TILE_COUNT,
        BPP,
        BMF_CLEAR | BMF_INTERLEAVED
    );

    for (UBYTE t = 0; t < TILE_COUNT; ++t) {
        UWORD base = TILE_SIZE * t;
        blitRect(tileSet, 0, base,     TILE_SIZE, TILE_SIZE, t);     /* base colour  */
        blitRect(tileSet, 0, base + 4, TILE_SIZE, 2,         t + 5); /* horiz band   */
        blitRect(tileSet, 4, base,     2,         TILE_SIZE, t + 5); /* vert stripe  */
    }
}

/* ----------------------------------------------------------- map generator */

static UBYTE tileAt(UWORD x, UWORD y) {
    UWORD v = (x * 7u + y * 13u + (x ^ y) * 3u) & 0xFFu;
    if      (v < 80)  return TILE_GRASS;
    else if (v < 130) return TILE_DIRT;
    else if (v < 170) return TILE_SAND;
    else if (v < 210) return TILE_STONE;
    else              return TILE_WATER;
}

static void buildMap(void) {
    for (UWORD x = 0; x < MAP_TILES_X; ++x) {
        for (UWORD y = 0; y < MAP_TILES_Y; ++y) {
            tileBuf->pTileData[x][y] = tileAt(x, y);
        }
    }
}

/* ---------------------------------------------------- ACE state callbacks */

static void scrollerCreate(void) {
    view = viewCreate(0, TAG_DONE);

    vport = vPortCreate(0,
        TAG_VPORT_VIEW, view,
        TAG_VPORT_BPP,  BPP,
        TAG_DONE
    );

    for (UBYTE i = 0; i < (1 << BPP); ++i) {
        vport->pPalette[i] = palette[i];
    }

    buildTileSet();

    tileBuf = tileBufferCreate(0,
        TAG_TILEBUFFER_VPORT,               vport,
        TAG_TILEBUFFER_BITMAP_FLAGS,        BMF_CLEAR | BMF_INTERLEAVED,
        TAG_TILEBUFFER_BOUND_TILE_X,        MAP_TILES_X,
        TAG_TILEBUFFER_BOUND_TILE_Y,        MAP_TILES_Y,
        TAG_TILEBUFFER_TILE_SHIFT,          TILE_SHIFT,
        TAG_TILEBUFFER_TILESET,             tileSet,
        TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
        TAG_DONE
    );

    buildMap();
    cameraSetCoord(tileBuf->pCamera, 0, 0);
    tileBufferRedrawAll(tileBuf);

    viewLoad(view);
    systemUnuse();
}

static void scrollerLoop(void) {
    WORD dx = 0, dy = 0;
    if (keyCheck(KEY_LEFT)  || keyCheck(KEY_A)) { dx = -CAM_SPEED; }
    if (keyCheck(KEY_RIGHT) || keyCheck(KEY_D)) { dx =  CAM_SPEED; }
    if (keyCheck(KEY_UP)    || keyCheck(KEY_W)) { dy = -CAM_SPEED; }
    if (keyCheck(KEY_DOWN)  || keyCheck(KEY_S)) { dy =  CAM_SPEED; }

    if (keyUse(KEY_ESCAPE)) {
        gameExit();
        return;
    }

    cameraMoveBy(tileBuf->pCamera, dx, dy);
    tileBufferProcess(tileBuf);
    viewProcessManagers(view);
    copProcessBlocks();
    vPortWaitForEnd(vport);
}

static void scrollerDestroy(void) {
    systemUse();
    viewLoad(0);
    viewDestroy(view);
    bitmapDestroy(tileSet);
}

/* -------------------------------------------------- exported state struct */

tState scrollerState = {
    .cbCreate  = scrollerCreate,
    .cbLoop    = scrollerLoop,
    .cbDestroy = scrollerDestroy,
    .cbSuspend = 0,
    .cbResume  = 0,
};
