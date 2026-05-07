#ifndef GAME_PLAYER_DODGE_H
#define GAME_PLAYER_DODGE_H

/**
 * Player dodge / roll (ported from original game/player_dodge.c).
 */

extern int g_dodgeTimer;
extern int g_dodgeCooldownTimer;
extern int g_dodgeDxFp;
extern int g_dodgeDyFp;

void playerDodgeCooldownTick(void);

/**
 * Returns 1 if dodge started.
 * When input is idle, dodge direction uses aimDx/aimDy (view-space: mouse minus player center).
 */
int playerDodgeTryInitiate(int inputDx, int inputDy, int aimDx, int aimDy);

/** Returns 1 if dodge finished this frame. Updates *vxFp/*vyFp on finish (residual carry). */
int playerDodgeProcessFrame(int *vxFp, int *vyFp);

/**
 * Call after move; returns 1 if dodge interrupted by blocking (sets *vxFp/*vyFp 0).
 * dx, dy: attempted integer steps this frame.
 */
int playerDodgeHandleCollision(
    int oldX,
    int oldY,
    int newX,
    int newY,
    int dx,
    int dy,
    int *vxFp,
    int *vyFp
);

#endif /* GAME_PLAYER_DODGE_H */
