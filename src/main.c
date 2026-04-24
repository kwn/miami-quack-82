/**
 * ace-scroller – minimal horizontal-scrolling demo using ACE (Amiga C Engine)
 *
 * Controls:
 *   LEFT / RIGHT arrow  – scroll the camera
 *   ESC                 – quit
 *
 * Map layout  (100 × 16 tiles, each 16 × 16 px → 1600 × 256 world):
 *   - rows 0-12 : sky
 *   - rows 13-15: ground
 *   - every 10th column has a stone pillar from the ground up to row 5
 */

#undef GENERIC_MAIN_LOG_PATH
#include <ace/generic/main.h>

#include <ace/managers/key.h>
#include <ace/managers/game.h>
#include <ace/managers/blit.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/tilebuffer.h>
#include <ace/utils/extview.h>

/* ------------------------------------------------------------------ config */

#define BPP           4
#define TILE_SHIFT    4    /* 1 << 4 = 16 px */
#define TILE_SIZE    16

#define MAP_TILES_X 100
#define MAP_TILES_Y  16   /* 16 * 16 = 256 = screen height */

/* tile indices */
#define TILE_SKY     0
#define TILE_GROUND  1
#define TILE_STONE   2
#define TILE_COUNT   3

#define CAM_SPEED    3   /* pixels per frame – fast enough to feel responsive */

/* ----------------------------------------------------------------- palette */
static const UWORD s_pPalette[1 << BPP] = {
    [0]  = 0x0024,  /* sky – solid dark blue          */
    [1]  = 0x0136,  /* sky – slightly lighter (spare) */
    [2]  = 0x0555,  /* stone – dark gray              */
    [3]  = 0x0777,  /* stone – mid gray               */
    [4]  = 0x0999,  /* stone – light gray             */
    [5]  = 0x04C2,  /* grass – bright green           */
    [6]  = 0x02A0,  /* grass – mid green              */
    [7]  = 0x0180,  /* grass – dark green             */
    [8]  = 0x0A62,  /* earth – light soil             */
    [9]  = 0x0741,  /* earth – mid brown              */
    [10] = 0x0530,  /* earth – dark brown             */
    [11] = 0x0FF0,  /* yellow  (pillar highlight)     */
    [12] = 0x0FA0,  /* orange  (spare)                */
    [13] = 0x0F00,  /* red     (spare)                */
    [14] = 0x0CDE,  /* light cyan (spare)             */
    [15] = 0x0FFF,  /* white                          */
};

/* ------------------------------------------------------------------ state */
static tView              *s_pView;
static tVPort             *s_pVPort;
static tTileBufferManager *s_pTileBuf;
static tBitMap            *s_pTileSet;

/* ---------------------------------------------------------------- tileset */

/**
 * All tiles are in a single-column bitmap, each TILE_SIZE pixels tall.
 * Tile N starts at y = N * TILE_SIZE.
 */
static void buildTileSet(void) {
    s_pTileSet = bitmapCreate(
        TILE_SIZE,
        TILE_SIZE * TILE_COUNT,
        BPP,
        BMF_CLEAR | BMF_INTERLEAVED
    );

    /* ---- TILE 0: sky – solid dark blue, no gradient so it won't look like
     *              copper raster bars and won't hide horizontal movement ---- */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_SKY, TILE_SIZE, TILE_SIZE, 0);

    /* ---- TILE 1: ground – 3-layer (grass / mid-earth / deep-earth) ------- */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_GROUND,      TILE_SIZE,  2,  5); /* bright grass */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_GROUND +  2, TILE_SIZE,  2,  6); /* mid grass    */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_GROUND +  4, TILE_SIZE,  2,  7); /* dark grass   */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_GROUND +  6, TILE_SIZE,  4,  8); /* light soil   */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_GROUND + 10, TILE_SIZE,  6,  9); /* mid earth    */

    /* ---- TILE 2: stone pillar – horizontal "mortar" lines on gray base --- */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_STONE,       TILE_SIZE, TILE_SIZE, 3); /* base gray */
    /* mortar joints every 4 px */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_STONE +  3,  TILE_SIZE,  1, 2); /* dark joint  */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_STONE +  7,  TILE_SIZE,  1, 2);
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_STONE + 11,  TILE_SIZE,  1, 2);
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_STONE + 15,  TILE_SIZE,  1, 2);
    /* bright cap at top of each stone tile */
    blitRect(s_pTileSet, 0, TILE_SIZE * TILE_STONE,        TILE_SIZE,  1, 4); /* light top   */
}

/* ---------------------------------------------------------------- map */

/**
 * Landscape:
 *   - sky fills everything above ground
 *   - ground = bottom 3 rows
 *   - stone pillars at every 10th column, from y=5 down to ground
 *   - a "tower cap" (yellow) at y=4 on pillar columns (to make the top pop)
 */
static void buildMap(void) {
    for (UWORD x = 0; x < MAP_TILES_X; ++x) {
        UBYTE isPillarCol = ((x % 10) == 0);

        for (UWORD y = 0; y < MAP_TILES_Y; ++y) {
            UBYTE tile;

            if (y >= MAP_TILES_Y - 3) {
                /* bottom 3 rows: always ground */
                tile = TILE_GROUND;
            } else if (isPillarCol && y >= 5) {
                /* pillar body: stone from row 5 down to just above ground */
                tile = TILE_STONE;
            } else {
                tile = TILE_SKY;
            }

            s_pTileBuf->pTileData[x][y] = tile;
        }
    }
}

/* --------------------------------------------------------- ACE callbacks */

void genericCreate(void) {
    keyCreate();

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
        TAG_TILEBUFFER_REDRAW_QUEUE_LENGTH, 50,
        TAG_DONE
    );

    buildMap();
    cameraSetCoord(s_pTileBuf->pCamera, 0, 0);
    tileBufferRedrawAll(s_pTileBuf);

    viewLoad(s_pView);
    systemUnuse();
}

void genericProcess(void) {
    keyProcess();

    if (keyUse(KEY_ESCAPE)) {
        gameExit();
        return;
    }

    WORD wDx = 0;
    if (keyCheck(KEY_LEFT))  { wDx = -CAM_SPEED; }
    if (keyCheck(KEY_RIGHT)) { wDx =  CAM_SPEED;  }

    cameraMoveBy(s_pTileBuf->pCamera, wDx, 0);

    tileBufferProcess(s_pTileBuf);
    viewProcessManagers(s_pView);
    copProcessBlocks();
    vPortWaitForEnd(s_pVPort);
}

void genericDestroy(void) {
    systemUse();

    viewLoad(0);
    viewDestroy(s_pView);
    bitmapDestroy(s_pTileSet);

    keyDestroy();
}
