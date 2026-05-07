#include "player_anim.h"

typedef struct {
    const int *frameOffsets;
    int frameCount;
    int ticksPerFrame;
} AnimationDef;

static const int idleFrameOffsets[] = {9, 12};
static const int walkFrameOffsets[] = {0, 3, 6, 3};
static const int dodgeFrameOffsets[] = {0, 6};

static const AnimationDef animations[] = {
    [PLAYER_ANIM_IDLE] = {idleFrameOffsets, 2, 25},
    [PLAYER_ANIM_WALK] = {walkFrameOffsets, 4, 4},
    [PLAYER_ANIM_DODGE] = {dodgeFrameOffsets, 4, 2},
};

static PlayerAnimState currentState;
static int tickCounter;
static int currentFrame;

void playerAnimCreate(void) {
    currentState = PLAYER_ANIM_IDLE;
    tickCounter = 0;
    currentFrame = 0;
}

void playerAnimProcess(int inputDx, int inputDy, int isDodging) {
    PlayerAnimState newState;

    if (isDodging) {
        newState = PLAYER_ANIM_DODGE;
    } else if (inputDx || inputDy) {
        newState = PLAYER_ANIM_WALK;
    } else {
        newState = PLAYER_ANIM_IDLE;
    }

    if (newState != currentState) {
        currentState = newState;
        tickCounter = 0;
        currentFrame = 0;
        return;
    }

    ++tickCounter;
    if (tickCounter >= animations[currentState].ticksPerFrame) {
        tickCounter = 0;
        ++currentFrame;
        if (currentFrame >= animations[currentState].frameCount) {
            currentFrame = 0;
        }
    }
}

int playerAnimGetFrameOffset(void) {
    return animations[currentState].frameOffsets[currentFrame];
}
