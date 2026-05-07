#ifndef GAME_PLAYER_ANIM_H
#define GAME_PLAYER_ANIM_H

typedef enum {
    PLAYER_ANIM_IDLE,
    PLAYER_ANIM_WALK,
    PLAYER_ANIM_DODGE
} PlayerAnimState;

void playerAnimCreate(void);
void playerAnimProcess(int inputDx, int inputDy, int isDodging);
int playerAnimGetFrameOffset(void);

#endif /* GAME_PLAYER_ANIM_H */
