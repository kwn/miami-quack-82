#include "scroller.h"

#include "game.h"
#include "map.h"
#include "map_generator.h"

#include <ace/managers/blit.h>
#include <ace/managers/game.h>
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/utils/bitmap.h>

/* ------------------------------------------------------------------ sizes */

#define TILE_SHIFT     4   /* 1 << 4 = 16 px per tile */
#define TILE_SIZE     16

/* ------------------------------------------------------------------ state */

static tVPort             *gameVport;
static tTileBufferManager *tileBuf;
static tBitMap            *tileSet;
static tBitMap            *pristineBuffer;

static void onTileDraw(UWORD tileX, UWORD tileY, tBitMap *bitmap, UWORD bitmapX, UWORD bitmapY) {
    (void)tileX;
    (void)tileY;
    blitCopyAligned(bitmap, bitmapX, bitmapY, pristineBuffer, bitmapX, bitmapY, TILE_SIZE, TILE_SIZE);
}

static void buildMap(void) {
    mapGeneratorSetSeed(0x19820827);
    mapGeneratorGenerateOutdoor();

    for (UWORD x = 0; x < MAP_W; ++x) {
        for (UWORD y = 0; y < MAP_H; ++y) {
            tileBuf->pTileData[x][y] = mapData[y * MAP_W + x];
        }
    }
}

/* --------------------------------------------------------------- scroller */

static UWORD clampCameraX(UWORD x) {
    UWORD maxX = (MAP_W * TILE_SIZE) - GAME_SCREEN_W;
    return x > maxX ? maxX : x;
}

static UWORD clampCameraY(UWORD y) {
    UWORD maxY = (MAP_H * TILE_SIZE) - GAME_VIEW_H;
    return y > maxY ? maxY : y;
}

tVPort *scrollerCreate(tView *view) {
    gameVport = vPortCreate(0,
        TAG_VPORT_VIEW, view,
        TAG_VPORT_BPP,  GAME_BPP,
        TAG_VPORT_WIDTH, GAME_SCREEN_W,
        TAG_VPORT_HEIGHT, GAME_VIEW_H,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VPORT_USES_AGA, 1,
        TAG_VPORT_FMODE, GAME_AGA_FMODE,
#endif
        TAG_DONE
    );

    tileSet = bitmapCreateFromPath("data/tileset/outdoor.bm", 0);

    tileBuf = tileBufferCreate(0,
        TAG_TILEBUFFER_VPORT,               gameVport,
        TAG_TILEBUFFER_BITMAP_FLAGS,        BMF_CLEAR | BMF_INTERLEAVED,
        TAG_TILEBUFFER_IS_DBLBUF,           1,
        TAG_TILEBUFFER_BOUND_TILE_X,        MAP_W,
        TAG_TILEBUFFER_BOUND_TILE_Y,        MAP_H,
        TAG_TILEBUFFER_TILE_SHIFT,          TILE_SHIFT,
        TAG_TILEBUFFER_TILESET,             tileSet,
        TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 100,
        TAG_TILEBUFFER_CALLBACK_TILE_DRAW,  onTileDraw,
        TAG_DONE
    );
    pristineBuffer = bitmapCreate(
        bitmapGetByteWidth(tileBuf->pScroll->pBack) * 8,
        tileBuf->pScroll->pBack->Rows,
        GAME_BPP,
        BMF_CLEAR | BMF_INTERLEAVED
    );

    buildMap();
    cameraSetCoord(tileBuf->pCamera, 0, 0);

    return gameVport;
}

void scrollerMoveCamera(WORD dx, WORD dy) {
    cameraMoveBy(tileBuf->pCamera, dx, dy);
}

void scrollerSetCamera(UWORD x, UWORD y) {
    cameraSetCoord(tileBuf->pCamera, clampCameraX(x), clampCameraY(y));
}

void scrollerRedrawAll(void) {
    tileBufferRedrawAll(tileBuf);
}

UWORD scrollerGetCameraX(void) {
    return tileBuf->pCamera->uPos.uwX;
}

UWORD scrollerGetCameraY(void) {
    return tileBuf->pCamera->uPos.uwY;
}

tBitMap *scrollerGetFrontBuffer(void) {
    return tileBuf->pScroll->pFront;
}

tBitMap *scrollerGetBackBuffer(void) {
    return tileBuf->pScroll->pBack;
}

tBitMap *scrollerGetPristineBuffer(void) {
    return pristineBuffer;
}

UWORD scrollerGetBufferAvailHeight(void) {
    return tileBuf->pScroll->uwBmAvailHeight;
}

UWORD scrollerGetWorldWidth(void) {
    return MAP_W * TILE_SIZE;
}

UWORD scrollerGetWorldHeight(void) {
    return MAP_H * TILE_SIZE;
}

void scrollerDestroy(void) {
    bitmapDestroy(pristineBuffer);
    bitmapDestroy(tileSet);
    gameVport = 0;
    tileBuf = 0;
    tileSet = 0;
    pristineBuffer = 0;
}
