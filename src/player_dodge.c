#include "player_dodge.h"

#include "math_utils.h"
#include "player_physics.h"

#define DODGE_PEAK_SPEED_FP ((PLAYER_PHYS_MAX_SPEED_FP * 5) / 2)
#define DODGE_DURATION_FRAMES 26
#define DODGE_COOLDOWN_FRAMES 30

static const int dodgeSineLut[DODGE_DURATION_FRAMES] = {
    64, 91, 118, 143, 167, 189, 209, 225, 238, 248, 253, 256,
    254, 249, 242, 231, 218, 202, 184, 163, 141, 116, 91, 64, 37, 10
};

int g_dodgeTimer = 0;
int g_dodgeCooldownTimer = 0;
int g_dodgeDxFp = 0;
int g_dodgeDyFp = 0;

static int dodgeBaseDxFp = 0;
static int dodgeBaseDyFp = 0;

void playerDodgeCooldownTick(void) {
    if (g_dodgeCooldownTimer > 0) {
        --g_dodgeCooldownTimer;
    }
}

int playerDodgeTryInitiate(int inputDx, int inputDy, int aimDx, int aimDy) {
    if (g_dodgeCooldownTimer > 0) {
        return 0;
    }

    int ddx = inputDx;
    int ddy = inputDy;

    if (ddx == 0 && ddy == 0) {
        int dxMouse = aimDx;
        int dyMouse = aimDy;
        int len = mathApproxVectorLength(dxMouse, dyMouse);
        if (len > 0) {
            dodgeBaseDxFp = (dxMouse * DODGE_PEAK_SPEED_FP) / len;
            dodgeBaseDyFp = (dyMouse * DODGE_PEAK_SPEED_FP) / len;
            int actualLen = mathApproxVectorLength(dodgeBaseDxFp, dodgeBaseDyFp);
            if (actualLen > 0) {
                dodgeBaseDxFp = (dodgeBaseDxFp * DODGE_PEAK_SPEED_FP) / actualLen;
                dodgeBaseDyFp = (dodgeBaseDyFp * DODGE_PEAK_SPEED_FP) / actualLen;
            }
        } else {
            dodgeBaseDxFp = DODGE_PEAK_SPEED_FP;
            dodgeBaseDyFp = 0;
        }
    } else {
        int len = mathApproxVectorLength(ddx, ddy);
        if (len > 0) {
            dodgeBaseDxFp = (ddx * DODGE_PEAK_SPEED_FP) / len;
            dodgeBaseDyFp = (ddy * DODGE_PEAK_SPEED_FP) / len;
            int actualLen = mathApproxVectorLength(dodgeBaseDxFp, dodgeBaseDyFp);
            if (actualLen > 0) {
                dodgeBaseDxFp = (dodgeBaseDxFp * DODGE_PEAK_SPEED_FP) / actualLen;
                dodgeBaseDyFp = (dodgeBaseDyFp * DODGE_PEAK_SPEED_FP) / actualLen;
            }
        } else {
            dodgeBaseDxFp = DODGE_PEAK_SPEED_FP;
            dodgeBaseDyFp = 0;
        }
    }

    g_dodgeTimer = DODGE_DURATION_FRAMES;
    {
        int curveVal = dodgeSineLut[0];
        g_dodgeDxFp = (dodgeBaseDxFp * curveVal) >> 8;
        g_dodgeDyFp = (dodgeBaseDyFp * curveVal) >> 8;
    }
    return 1;
}

int playerDodgeProcessFrame(int *vxFp, int *vyFp) {
    --g_dodgeTimer;
    if (g_dodgeTimer <= 0) {
        g_dodgeCooldownTimer = DODGE_COOLDOWN_FRAMES;
        *vxFp = (dodgeBaseDxFp * 10) >> 8;
        *vyFp = (dodgeBaseDyFp * 10) >> 8;
        return 1;
    }

    {
        int frameIdx = DODGE_DURATION_FRAMES - g_dodgeTimer;
        if (frameIdx >= 0 && frameIdx < DODGE_DURATION_FRAMES) {
            int curveVal = dodgeSineLut[frameIdx];
            g_dodgeDxFp = (dodgeBaseDxFp * curveVal) >> 8;
            g_dodgeDyFp = (dodgeBaseDyFp * curveVal) >> 8;
        }
    }
    return 0;
}

int playerDodgeHandleCollision(
    int oldX,
    int oldY,
    int newX,
    int newY,
    int dx,
    int dy,
    int *vxFp,
    int *vyFp
) {
    if ((newX == oldX && dx != 0) || (newY == oldY && dy != 0)) {
        g_dodgeTimer = 0;
        g_dodgeCooldownTimer = DODGE_COOLDOWN_FRAMES;
        *vxFp = 0;
        *vyFp = 0;
        return 1;
    }
    return 0;
}
