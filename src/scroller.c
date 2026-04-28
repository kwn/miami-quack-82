#include "scroller.h"

#include "game.h"

#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/viewport/tilebuffer.h>

/* ------------------------------------------------------------------ sizes */

#define TILE_SHIFT     4   /* 1 << 4 = 16 px per tile */
#define TILE_SIZE     16

#define MAP_TILES_X  128
#define MAP_TILES_Y  128

/* ------------------------------------------------------------------ tiles */

#define TILE_GRASS   0
#define TILE_DIRT    1
#define TILE_WATER   2
#define TILE_STONE   3
#define TILE_SAND    4
#define TILE_COUNT   5

/* ------------------------------------------------------------------ state */

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

/* --------------------------------------------------------------- scroller */

tVPort *scrollerCreate(tView *view) {
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

    return gameVport;
}

void scrollerMoveCamera(WORD dx, WORD dy) {
    cameraMoveBy(tileBuf->pCamera, dx, dy);
}

void scrollerSetCamera(UWORD x, UWORD y) {
    cameraSetCoord(tileBuf->pCamera, x, y);
}

UWORD scrollerGetCameraX(void) {
    return tileBuf->pCamera->uPos.uwX;
}

UWORD scrollerGetCameraY(void) {
    return tileBuf->pCamera->uPos.uwY;
}

void scrollerProcess(void) {
    tileBufferProcess(tileBuf);
}

void scrollerDestroy(void) {
    bitmapDestroy(tileSet);
    gameVport = 0;
    tileBuf = 0;
    tileSet = 0;
}
