#ifndef GAME_PLAYER_PHYSICS_H
#define GAME_PLAYER_PHYSICS_H

#include <ace/types.h>

#define PLAYER_PHYS_FP_SHIFT 8
#define PLAYER_PHYS_MAX_SPEED_FP (2 << PLAYER_PHYS_FP_SHIFT)

void playerPhysicsVelocityToSteps(int vxFp, int vyFp, WORD *outStepX, WORD *outStepY);

#endif /* GAME_PLAYER_PHYSICS_H */
