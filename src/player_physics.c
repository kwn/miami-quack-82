#include "player_physics.h"

void playerPhysicsVelocityToSteps(int vxFp, int vyFp, WORD *outStepX, WORD *outStepY) {
    WORD sx;
    WORD sy;

    if (vxFp > 0) {
        sx = (WORD)((vxFp + 128) >> PLAYER_PHYS_FP_SHIFT);
    } else if (vxFp < 0) {
        sx = (WORD)-((-vxFp + 128) >> PLAYER_PHYS_FP_SHIFT);
    } else {
        sx = 0;
    }

    if (vyFp > 0) {
        sy = (WORD)((vyFp + 128) >> PLAYER_PHYS_FP_SHIFT);
    } else if (vyFp < 0) {
        sy = (WORD)-((-vyFp + 128) >> PLAYER_PHYS_FP_SHIFT);
    } else {
        sy = 0;
    }

    *outStepX = sx;
    *outStepY = sy;
}
