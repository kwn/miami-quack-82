#include "game_camera.h"

#include "aim.h"
#include "game.h"
#include "scroller.h"

#define TILE_HALF_W 8
#define TILE_HALF_H 8
#define LOOKAHEAD_DIVISOR 2
#define MAX_SCROLLS_PER_FRAME 4
#define INITIAL_FOCUS_X (GAME_SCREEN_W / 2)
#define INITIAL_FOCUS_Y (GAME_VIEW_H / 2)

static LONG focusXfp;
static LONG focusYfp;

static WORD clampCameraDelta(LONG delta) {
    if (delta > MAX_SCROLLS_PER_FRAME) {
        return MAX_SCROLLS_PER_FRAME;
    }
    if (delta < -MAX_SCROLLS_PER_FRAME) {
        return -MAX_SCROLLS_PER_FRAME;
    }
    return (WORD)delta;
}

static void calculateTarget(LONG *outX, LONG *outY) {
    LONG aimOffsetX = (LONG)aimGetX() - (GAME_SCREEN_W / 2);
    LONG aimOffsetY = (LONG)aimGetY() - (GAME_VIEW_H / 2);
    LONG lookaheadX = aimOffsetX / LOOKAHEAD_DIVISOR;
    LONG lookaheadY = aimOffsetY / LOOKAHEAD_DIVISOR;

    *outX = (focusXfp >> 8) - (GAME_SCREEN_W / 2) + TILE_HALF_W + lookaheadX;
    *outY = (focusYfp >> 8) - (GAME_VIEW_H / 2) + TILE_HALF_H + lookaheadY;

    if (*outX < 0) {
        *outX = 0;
    }
    if (*outY < 0) {
        *outY = 0;
    }
}

void gameCameraCreate(void) {
    gameCameraSetFocus(INITIAL_FOCUS_X, INITIAL_FOCUS_Y);
    scrollerSetCamera(0, 0);
}

void gameCameraSetFocus(UWORD worldX, UWORD worldY) {
    focusXfp = (LONG)worldX << 8;
    focusYfp = (LONG)worldY << 8;
}

void gameCameraProcess(void) {
    LONG targetX;
    LONG targetY;
    UWORD currentX = scrollerGetCameraX();
    UWORD currentY = scrollerGetCameraY();
    WORD deltaX;
    WORD deltaY;

    calculateTarget(&targetX, &targetY);
    deltaX = clampCameraDelta(targetX - currentX);
    deltaY = clampCameraDelta(targetY - currentY);

    scrollerSetCamera(currentX + deltaX, currentY + deltaY);
}

void gameCameraDestroy(void) {
    focusXfp = 0;
    focusYfp = 0;
}
