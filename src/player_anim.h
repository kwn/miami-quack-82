#ifndef PLAYER_ANIM_H
#define PLAYER_ANIM_H

typedef enum {
    PLAYER_ANIM_IDLE,
    PLAYER_ANIM_WALK
} PlayerAnimState;

void playerAnimCreate(void);
void playerAnimProcess(int inputDx, int inputDy);
int playerAnimGetFrameOffset(void);

#endif /* PLAYER_ANIM_H */
