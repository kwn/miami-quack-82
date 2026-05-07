#include "player.h"

#include "aim.h"
#include "math_utils.h"
#include "player_anim.h"
#include "player_dodge.h"
#include "player_input.h"
#include "player_physics.h"
#include "scroller.h"

#include <ace/managers/bob.h>
#include <ace/managers/key.h>
#include <ace/utils/bitmap.h>

#define PLAYER_W 16
#define PLAYER_H 16
#define PLAYER_FRAME_COUNT 16
#define PLAYER_DIRECTION_RIGHT 0
#define PLAYER_DIRECTION_LEFT 1
#define PLAYER_DIRECTION_UP 2

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
static UBYTE playerIsDodging;

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

    if (velocity > PLAYER_PHYS_MAX_SPEED_FP) {
        velocity = PLAYER_PHYS_MAX_SPEED_FP;
    }
    if (velocity < -PLAYER_PHYS_MAX_SPEED_FP) {
        velocity = -PLAYER_PHYS_MAX_SPEED_FP;
    }

    return velocity;
}

static void updateFacingFromAim(void) {
    WORD playerViewX = (WORD)playerX - (WORD)scrollerGetCameraX();
    WORD playerViewY = (WORD)playerY - (WORD)scrollerGetCameraY();
    WORD dx = (WORD)aimGetX() - playerViewX;
    WORD dy = (WORD)aimGetY() - playerViewY;

    if (dx || dy) {
        playerDirection = (UBYTE)mathDirection3FromVector((int)dx, (int)dy);
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
    UWORD frame = (UWORD)playerAnimGetFrameOffset() + playerDirection;
    bobSetFrame(&playerBob, playerFramePtrs[frame], playerMaskPtrs[frame]);
}

/**
 * Normal movement: clamps to world rect and zeros velocity when hitting an edge.
 */
static void applyMovementNormal(WORD stepX, WORD stepY) {
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

/**
 * Dodge roll: same world clamp, does not modify velocity (dodge uses its own FP speeds).
 */
static void applyMovementDodge(WORD stepX, WORD stepY, UWORD *oldX, UWORD *oldY) {
    LONG nextX = (LONG)playerX + stepX;
    LONG nextY = (LONG)playerY + stepY;
    UWORD maxX = scrollerGetWorldWidth() - PLAYER_W;
    UWORD maxY = scrollerGetWorldHeight() - PLAYER_H;

    *oldX = playerX;
    *oldY = playerY;

    if (nextX < 0) {
        nextX = 0;
    }
    if (nextY < 0) {
        nextY = 0;
    }
    if (nextX > maxX) {
        nextX = maxX;
    }
    if (nextY > maxY) {
        nextY = maxY;
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
    playerIsDodging = 0;
    g_dodgeTimer = 0;
    g_dodgeCooldownTimer = 0;
    playerAnimCreate();

    bobManagerCreate(
        scrollerGetFrontBuffer(),
        scrollerGetBackBuffer(),
        scrollerGetPristineBuffer(),
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

    playerDodgeCooldownTick();

    {
        WORD camX = scrollerGetCameraX();
        WORD camY = scrollerGetCameraY();
        int aimDx = (int)aimGetX() - ((int)playerX - (int)camX + (PLAYER_W / 2));
        int aimDy = (int)aimGetY() - ((int)playerY - (int)camY + (PLAYER_H / 2));

        if (!playerIsDodging && keyCheck(KEY_SPACE)) {
            if (playerDodgeTryInitiate(inputDx, inputDy, aimDx, aimDy)) {
                playerIsDodging = 1;
                velocityXfp = 0;
                velocityYfp = 0;
            }
        }
    }

    if (playerIsDodging) {
        int vxTmp = (int)velocityXfp;
        int vyTmp = (int)velocityYfp;

        if (playerDodgeProcessFrame(&vxTmp, &vyTmp)) {
            playerIsDodging = 0;
            velocityXfp = (WORD)vxTmp;
            velocityYfp = (WORD)vyTmp;
        } else {
            velocityXfp = 0;
            velocityYfp = 0;
        }

        playerPhysicsVelocityToSteps(g_dodgeDxFp, g_dodgeDyFp, &stepX, &stepY);

        {
            UWORD oldX;
            UWORD oldY;
            applyMovementDodge(stepX, stepY, &oldX, &oldY);
            if (playerDodgeHandleCollision(
                    (int)oldX,
                    (int)oldY,
                    (int)playerX,
                    (int)playerY,
                    (int)stepX,
                    (int)stepY,
                    &vxTmp,
                    &vyTmp
                )) {
                playerIsDodging = 0;
                velocityXfp = (WORD)vxTmp;
                velocityYfp = (WORD)vyTmp;
            }
        }

        playerDirection = (UBYTE)mathDirection3FromVector(g_dodgeDxFp, g_dodgeDyFp);
        playerAnimProcess(inputDx, inputDy, 1);
        setCurrentFrame();
        return;
    }

    velocityXfp = applyAxisPhysics(velocityXfp, inputDx);
    velocityYfp = applyAxisPhysics(velocityYfp, inputDy);

    playerPhysicsVelocityToSteps((int)velocityXfp, (int)velocityYfp, &stepX, &stepY);
    applyMovementNormal(stepX, stepY);

    playerAnimProcess(inputDx, inputDy, 0);
    updateFacingFromAim();
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
    playerIsDodging = 0;
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

UBYTE playerIsInDodge(void) {
    return playerIsDodging;
}
