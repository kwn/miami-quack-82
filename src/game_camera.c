#include "game_camera.h"

#include "aim.h"
#include "game.h"
#include "scroller.h"

#define TILE_HALF_W 8
#define TILE_HALF_H 8
#define LOOKAHEAD_DIVISOR 2
#define MAX_SCROLLS_PER_FRAME 4
#define CAMERA_DEADZONE_X 40
#define CAMERA_DEADZONE_Y 28
#define FOCUS_RETURN_DIVISOR_SHIFT 4
#define FOCUS_MAX_STEPS_PER_FRAME 2
#define FOCUS_MAX_STEP_FP (FOCUS_MAX_STEPS_PER_FRAME << 8)
#define INITIAL_FOCUS_X (GAME_SCREEN_W / 2)
#define INITIAL_FOCUS_Y (GAME_VIEW_H / 2)

static LONG focusXfp;
static LONG focusYfp;
static UWORD playerX;
static UWORD playerY;
static UBYTE isPlayerMoving;

static WORD clampCameraDelta(LONG delta) {
    if (delta > MAX_SCROLLS_PER_FRAME) {
        return MAX_SCROLLS_PER_FRAME;
    }
    if (delta < -MAX_SCROLLS_PER_FRAME) {
        return -MAX_SCROLLS_PER_FRAME;
    }
    return (WORD)delta;
}

static LONG absLong(LONG value) {
    return value < 0 ? -value : value;
}

static void updateFocusDeadzone(void) {
    LONG focusX = focusXfp >> 8;
    LONG focusY = focusYfp >> 8;
    LONG dx = (LONG)playerX - focusX;
    LONG dy = (LONG)playerY - focusY;

    if (dx > CAMERA_DEADZONE_X) {
        focusX = (LONG)playerX - CAMERA_DEADZONE_X;
    }
    if (dx < -CAMERA_DEADZONE_X) {
        focusX = (LONG)playerX + CAMERA_DEADZONE_X;
    }
    if (dy > CAMERA_DEADZONE_Y) {
        focusY = (LONG)playerY - CAMERA_DEADZONE_Y;
    }
    if (dy < -CAMERA_DEADZONE_Y) {
        focusY = (LONG)playerY + CAMERA_DEADZONE_Y;
    }

    focusXfp = focusX << 8;
    focusYfp = focusY << 8;
}

static void smoothFocusToPlayer(void) {
    LONG targetXfp;
    LONG targetYfp;
    LONG diffX;
    LONG diffY;
    LONG stepX;
    LONG stepY;
    LONG absStepX;
    LONG absStepY;
    LONG maxStep;

    if (isPlayerMoving) {
        return;
    }

    targetXfp = (LONG)playerX << 8;
    targetYfp = (LONG)playerY << 8;
    diffX = targetXfp - focusXfp;
    diffY = targetYfp - focusYfp;
    stepX = diffX >> FOCUS_RETURN_DIVISOR_SHIFT;
    stepY = diffY >> FOCUS_RETURN_DIVISOR_SHIFT;
    absStepX = absLong(stepX);
    absStepY = absLong(stepY);
    maxStep = absStepX > absStepY ? absStepX : absStepY;

    if (maxStep > FOCUS_MAX_STEP_FP) {
        stepX = (stepX * FOCUS_MAX_STEP_FP) / maxStep;
        stepY = (stepY * FOCUS_MAX_STEP_FP) / maxStep;
    }

    if (absStepX == 0 && diffX != 0) {
        stepX = diffX > 0 ? 1 : -1;
    }
    if (absStepY == 0 && diffY != 0) {
        stepY = diffY > 0 ? 1 : -1;
    }

    focusXfp += stepX;
    focusYfp += stepY;
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
    playerX = worldX;
    playerY = worldY;
}

void gameCameraTrackPlayer(UWORD worldX, UWORD worldY, UBYTE isMoving) {
    playerX = worldX;
    playerY = worldY;
    isPlayerMoving = isMoving;
}

void gameCameraSnapToFocus(void) {
    LONG targetX;
    LONG targetY;

    calculateTarget(&targetX, &targetY);
    scrollerSetCamera((UWORD)targetX, (UWORD)targetY);
}

void gameCameraProcess(void) {
    LONG targetX;
    LONG targetY;
    UWORD currentX = scrollerGetCameraX();
    UWORD currentY = scrollerGetCameraY();
    WORD deltaX;
    WORD deltaY;

    updateFocusDeadzone();
    smoothFocusToPlayer();
    calculateTarget(&targetX, &targetY);
    deltaX = clampCameraDelta(targetX - currentX);
    deltaY = clampCameraDelta(targetY - currentY);

    scrollerSetCamera(currentX + deltaX, currentY + deltaY);
}

void gameCameraDestroy(void) {
    focusXfp = 0;
    focusYfp = 0;
    playerX = 0;
    playerY = 0;
    isPlayerMoving = 0;
}
