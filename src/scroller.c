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

#include "campaign.h"
#include "hud.h"

#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/system.h>
#include <ace/managers/state.h>
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/utils/extview.h>

extern tStateManager *stateMgr;

/* ------------------------------------------------------------------ sizes */

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

/* ------------------------------------------------------------------ state */

static tView              *view;
static tVPort             *gameVport;
static tTileBufferManager *tileBuf;
static tBitMap            *tileSet;

/* --------------------------------------------------------- tileset builder */

static void buildTileSet(void) {
    tileSet = bitmapCreate(
        TILE_SIZE,
        TILE_SIZE * TILE_COUNT,
        GAME_BPP,
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
    view = viewCreate(0,
        TAG_VIEW_GLOBAL_PALETTE, 1,
        TAG_VIEW_GLOBAL_BPP, 1,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VIEW_USES_AGA, 1,
#endif
        TAG_DONE
    );

    hudCreate(view);

    gameVport = vPortCreate(0,
        TAG_VPORT_VIEW, view,
        TAG_VPORT_BPP,  GAME_BPP,
        TAG_VPORT_WIDTH, GAME_SCREEN_W,
        TAG_VPORT_HEIGHT, GAME_VIEW_H,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VPORT_USES_AGA, 1,
        TAG_VPORT_FMODE, 0,
#endif
        TAG_DONE
    );

    buildTileSet();

    tileBuf = tileBufferCreate(0,
        TAG_TILEBUFFER_VPORT,               gameVport,
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
        campaignSetLevelResult(LEVEL_RESULT_EXIT_TO_TITLE);
        stateChange(stateMgr, &campaignState);
        return;
    }

    if (keyUse(KEY_RETURN) || keyUse(KEY_SPACE)) {
        campaignSetLevelResult(LEVEL_RESULT_COMPLETED);
        stateChange(stateMgr, &campaignState);
        return;
    }

    cameraMoveBy(tileBuf->pCamera, dx, dy);
    tileBufferProcess(tileBuf);
    viewProcessManagers(view);
    copProcessBlocks();
    vPortWaitForEnd(gameVport);
}

static void scrollerDestroy(void) {
    systemUse();
    viewLoad(0);
    viewDestroy(view);
    bitmapDestroy(tileSet);
    hudDestroy();
    view = 0;
    gameVport = 0;
    tileBuf = 0;
    tileSet = 0;
}

/* -------------------------------------------------- exported state struct */

tState scrollerState = {
    .cbCreate  = scrollerCreate,
    .cbLoop    = scrollerLoop,
    .cbDestroy = scrollerDestroy,
    .cbSuspend = 0,
    .cbResume  = 0,
};
