#include "player.h"

#include "aim.h"
#include "player_anim.h"
#include "player_input.h"
#include "scroller.h"

#include <ace/managers/bob.h>
#include <ace/utils/bitmap.h>

#define FP_SHIFT 8
#define PLAYER_W 16
#define PLAYER_H 16
#define PLAYER_FRAME_COUNT 16
#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT 1
#define PLAYER_DIRECTION_UP 2
#define PLAYER_MAX_SPEED_FP (2 << FP_SHIFT)
#define PLAYER_ACCEL_FP 128
#define PLAYER_FRICTION_FP 96

static tBitMap *playerBitmap;
static tBitMap *playerMask;
static tBob playerBob;
static UBYTE *playerFramePtrs[PLAYER_FRAME_COUNT];
static UBYTE *playerMaskPtrs[PLAYER_FRAME_COUNT];
static UWORD playerX;
static UWORD playerY;
static WORD velocityXfp;
static WORD velocityYfp;
static UBYTE playerDirection;
static UBYTE hasMovementInput;

static WORD applyAxisPhysics(WORD velocity, int input) {
    if (input != 0) {
        velocity += input > 0 ? PLAYER_ACCEL_FP : -PLAYER_ACCEL_FP;
    } else if (velocity > 0) {
        velocity -= PLAYER_FRICTION_FP;
        if (velocity < 0) {
            velocity = 0;
        }
    } else if (velocity < 0) {
        velocity += PLAYER_FRICTION_FP;
        if (velocity > 0) {
            velocity = 0;
        }
    }

    if (velocity > PLAYER_MAX_SPEED_FP) {
        velocity = PLAYER_MAX_SPEED_FP;
    }
    if (velocity < -PLAYER_MAX_SPEED_FP) {
        velocity = -PLAYER_MAX_SPEED_FP;
    }

    return velocity;
}

static WORD velocityToStep(WORD velocity) {
    if (velocity > 0) {
        return (velocity + 128) >> FP_SHIFT;
    }
    if (velocity < 0) {
        return -((-velocity + 128) >> FP_SHIFT);
    }
    return 0;
}

static UBYTE directionFromVector(WORD dx, WORD dy) {
    if (dx == 0 && dy == 0) {
        return PLAYER_DIRECTION_RIGHT;
    }

    if (dy < 0) {
        WORD absDx = dx < 0 ? -dx : dx;
        WORD absDy = -dy;
        if (absDx < ((absDy * 7) >> 2)) {
            return PLAYER_DIRECTION_UP;
        }
    }

    return dx >= 0 ? PLAYER_DIRECTION_RIGHT : PLAYER_DIRECTION_LEFT;
}

static void updateFacing(void) {
    WORD playerViewX = (WORD)playerX - (WORD)scrollerGetCameraX();
    WORD playerViewY = (WORD)playerY - (WORD)scrollerGetCameraY();
    WORD dx = (WORD)aimGetX() - playerViewX;
    WORD dy = (WORD)aimGetY() - playerViewY;

    if (dx || dy) {
        playerDirection = directionFromVector(dx, dy);
    }
}

static void buildFramePointers(void) {
    for (UWORD frame = 0; frame < PLAYER_FRAME_COUNT; ++frame) {
        UWORD offsetY = frame * PLAYER_H;
        playerFramePtrs[frame] = bobCalcFrameAddress(playerBitmap, offsetY);
        playerMaskPtrs[frame] = bobCalcFrameAddress(playerMask, offsetY);
    }
}

static void setCurrentFrame(void) {
    UWORD frame = playerAnimGetFrameOffset() + playerDirection;
    bobSetFrame(&playerBob, playerFramePtrs[frame], playerMaskPtrs[frame]);
}

static void applyMovement(WORD stepX, WORD stepY) {
    LONG nextX = (LONG)playerX + stepX;
    LONG nextY = (LONG)playerY + stepY;
    UWORD maxX = scrollerGetWorldWidth() - PLAYER_W;
    UWORD maxY = scrollerGetWorldHeight() - PLAYER_H;

    if (nextX < 0) {
        nextX = 0;
        velocityXfp = 0;
    }
    if (nextY < 0) {
        nextY = 0;
        velocityYfp = 0;
    }
    if (nextX > maxX) {
        nextX = maxX;
        velocityXfp = 0;
    }
    if (nextY > maxY) {
        nextY = maxY;
        velocityYfp = 0;
    }

    playerX = (UWORD)nextX;
    playerY = (UWORD)nextY;
}

void playerCreate(void) {
    playerBitmap = bitmapCreateFromPath("data/player/player.bm", 0);
    playerMask = bitmapCreateFromPath("data/player/player_mask.bm", 0);
    buildFramePointers();

    playerX = scrollerGetWorldWidth() / 2;
    playerY = scrollerGetWorldHeight() / 2;
    velocityXfp = 0;
    velocityYfp = 0;
    playerDirection = PLAYER_DIRECTION_RIGHT;
    hasMovementInput = 0;
    playerAnimCreate();

    bobManagerCreate(
        scrollerGetFrontBuffer(),
        scrollerGetBackBuffer(),
        scrollerGetBufferAvailHeight()
    );
    bobInit(
        &playerBob,
        PLAYER_W,
        PLAYER_H,
        1,
        playerFramePtrs[PLAYER_DIRECTION_RIGHT],
        playerMaskPtrs[PLAYER_DIRECTION_RIGHT],
        playerX,
        playerY
    );
    bobReallocateBuffers();
    bobDiscardUndraw();
}

void playerProcess(void) {
    int inputDx;
    int inputDy;
    WORD stepX;
    WORD stepY;

    playerInputGetMovement(&inputDx, &inputDy);
    hasMovementInput = (inputDx || inputDy);
    velocityXfp = applyAxisPhysics(velocityXfp, inputDx);
    velocityYfp = applyAxisPhysics(velocityYfp, inputDy);

    stepX = velocityToStep(velocityXfp);
    stepY = velocityToStep(velocityYfp);
    applyMovement(stepX, stepY);

    playerAnimProcess(inputDx, inputDy);
    updateFacing();
    setCurrentFrame();
}

void playerRender(void) {
    playerBob.sPos.uwX = playerX;
    playerBob.sPos.uwY = playerY;
    bobPush(&playerBob);
}

void playerDestroy(void) {
    bobManagerDestroy();

    if (playerMask) {
        bitmapDestroy(playerMask);
    }
    if (playerBitmap) {
        bitmapDestroy(playerBitmap);
    }

    playerBitmap = 0;
    playerMask = 0;
    playerX = 0;
    playerY = 0;
    velocityXfp = 0;
    velocityYfp = 0;
    playerDirection = PLAYER_DIRECTION_RIGHT;
    hasMovementInput = 0;
}

UWORD playerGetX(void) {
    return playerX;
}

UWORD playerGetY(void) {
    return playerY;
}

UBYTE playerHasMovementInput(void) {
    return hasMovementInput;
}
