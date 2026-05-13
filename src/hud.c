#include "hud.h"
#include "campaign.h"

#include <ace/managers/blit.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/palette.h>

static tVPort *hudVport;
static tSimpleBufferManager *hudBuffer;

static void loadPalette(void) {
    const WorldDefinition *world = campaignGetCurrentWorld();

#ifdef ACE_USE_AGA_FEATURES
    paletteLoadFromPath(world->palettePath, (UWORD *)hudVport->pPalette, GAME_COLOR_COUNT);
#else
    paletteLoadFromPath(world->palettePath, hudVport->pPalette, GAME_COLOR_COUNT);
#endif
}

static void drawBackground(void) {
    tBitMap *hudBitmap = bitmapCreateFromPath("data/bitmaps/hud.bm", 0);
    blitCopyAligned(hudBitmap, 0, 0, hudBuffer->pBack, 0, 0, GAME_SCREEN_W, HUD_H);
    blitWait();
    bitmapDestroy(hudBitmap);
}

tVPort *hudCreate(tView *view) {
    hudVport = vPortCreate(0,
        TAG_VPORT_VIEW, view,
        TAG_VPORT_BPP, GAME_BPP,
        TAG_VPORT_WIDTH, GAME_SCREEN_W,
        TAG_VPORT_HEIGHT, HUD_H,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VPORT_USES_AGA, 1,
        TAG_VPORT_FMODE, GAME_AGA_FMODE,
#endif
        TAG_DONE
    );

    hudBuffer = simpleBufferCreate(0,
        TAG_SIMPLEBUFFER_VPORT, hudVport,
        TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR,
        TAG_DONE
    );

    loadPalette();
    drawBackground();

    return hudVport;
}

void hudDestroy(void) {
    hudBuffer = 0;
    hudVport = 0;
}
