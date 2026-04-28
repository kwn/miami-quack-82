#include "aim.h"

#include "game.h"

#include <ace/managers/mouse.h>
#include <ace/managers/sprite.h>
#include <ace/managers/system.h>
#include <ace/utils/bitmap.h>

#include <string.h>

#define AIM_SPRITE_CHANNEL 0
#define AIM_SPRITE_W 16
#define AIM_SPRITE_H 16
#define AIM_SPRITE_METADATA_LINES 2
#define AIM_SPRITE_BITMAP_H (AIM_SPRITE_H + AIM_SPRITE_METADATA_LINES)
#define AIM_SPRITE_HOTSPOT_X 7
#define AIM_SPRITE_HOTSPOT_Y 7
#define AIM_TOP_CLIP_MAX AIM_SPRITE_HOTSPOT_Y
#define AIM_TOP_CLIP_COUNT (AIM_TOP_CLIP_MAX + 1)
#define AIM_BITMAP_CONTROL_ROW 1
#define AIM_MOUSE_PORT MOUSE_PORT_1
#define AIM_MOUSE_MIN_X 0
#define AIM_MOUSE_MAX_X (GAME_SCREEN_W - 1)
#define AIM_MOUSE_MIN_Y 0
#define AIM_MOUSE_MAX_Y (GAME_VIEW_H - 1)
#define AIM_INITIAL_X (GAME_SCREEN_W / 2)
#define AIM_INITIAL_Y (GAME_VIEW_H / 2)

static tBitMap *aimBitmap;
static tBitMap *aimTopClipBitmaps[AIM_TOP_CLIP_COUNT];
static tSprite *aimSprite;
static UWORD aimX;
static UWORD aimY;
static UBYTE currentTopClip;

static void copySpriteRow(tBitMap *source, UWORD sourceRow, tBitMap *dest, UWORD destRow) {
    memcpy(
        dest->Planes[0] + (destRow * dest->BytesPerRow),
        source->Planes[0] + (sourceRow * source->BytesPerRow),
        source->BytesPerRow
    );
}

static void buildTopClipBitmaps(void) {
    aimTopClipBitmaps[0] = aimBitmap;

    for (UBYTE topClip = 1; topClip < AIM_TOP_CLIP_COUNT; ++topClip) {
        UWORD visibleHeight = AIM_SPRITE_H - topClip;
        tBitMap *clippedBitmap = bitmapCreate(
            AIM_SPRITE_W,
            visibleHeight + AIM_SPRITE_METADATA_LINES,
            2,
            BMF_CLEAR | BMF_INTERLEAVED
        );

        for (UWORD row = 0; row < visibleHeight; ++row) {
            copySpriteRow(
                aimBitmap,
                AIM_BITMAP_CONTROL_ROW + topClip + row,
                clippedBitmap,
                AIM_BITMAP_CONTROL_ROW + row
            );
        }

        aimTopClipBitmaps[topClip] = clippedBitmap;
    }
}

static void updateSpritePosition(void) {
    UBYTE topClip = 0;
    if (aimY < AIM_SPRITE_HOTSPOT_Y) {
        topClip = AIM_SPRITE_HOTSPOT_Y - aimY;
    }

    if (topClip != currentTopClip) {
        spriteSetBitmap(aimSprite, aimTopClipBitmaps[topClip]);
        currentTopClip = topClip;
    }

    aimSprite->wX = (WORD)aimX - AIM_SPRITE_HOTSPOT_X;
    aimSprite->wY = topClip ? HUD_H : (WORD)(HUD_H + aimY) - AIM_SPRITE_HOTSPOT_Y;
    spriteRequestMetadataUpdate(aimSprite);
    spriteProcess(aimSprite);
}

void aimCreate(tView *view) {
    spriteManagerCreate(view, 0, 0);
    systemSetDmaBit(DMAB_SPRITE, 1);

    mouseCreate(AIM_MOUSE_PORT);
    mouseSetBounds(AIM_MOUSE_PORT, AIM_MOUSE_MIN_X, AIM_MOUSE_MIN_Y, AIM_MOUSE_MAX_X, AIM_MOUSE_MAX_Y);
    mouseSetPosition(AIM_MOUSE_PORT, AIM_INITIAL_X, AIM_INITIAL_Y);

    aimBitmap = bitmapCreateFromPath("data/sprites/aim.bm", 0);
    buildTopClipBitmaps();
    aimSprite = spriteAdd(AIM_SPRITE_CHANNEL, aimBitmap);
    spriteSetEnabled(aimSprite, 1);
    currentTopClip = 0;
    aimX = mouseGetX(AIM_MOUSE_PORT);
    aimY = mouseGetY(AIM_MOUSE_PORT);
    updateSpritePosition();
    spriteProcessChannel(AIM_SPRITE_CHANNEL);
}

void aimProcess(void) {
    mouseProcess();
    aimX = mouseGetX(AIM_MOUSE_PORT);
    aimY = mouseGetY(AIM_MOUSE_PORT);
    updateSpritePosition();
    spriteProcessChannel(AIM_SPRITE_CHANNEL);
}

void aimDestroy(void) {
    systemSetDmaBit(DMAB_SPRITE, 0);
    spriteManagerDestroy();
    mouseDestroy();

    for (UBYTE topClip = 1; topClip < AIM_TOP_CLIP_COUNT; ++topClip) {
        if (aimTopClipBitmaps[topClip]) {
            bitmapDestroy(aimTopClipBitmaps[topClip]);
            aimTopClipBitmaps[topClip] = 0;
        }
    }

    if (aimBitmap) {
        bitmapDestroy(aimBitmap);
    }

    aimTopClipBitmaps[0] = 0;
    aimBitmap = 0;
    aimSprite = 0;
    aimX = 0;
    aimY = 0;
    currentTopClip = 0;
}

UWORD aimGetX(void) {
    return aimX;
}

UWORD aimGetY(void) {
    return aimY;
}
