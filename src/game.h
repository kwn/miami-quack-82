#ifndef GAME_H
#define GAME_H

#include <ace/managers/state.h>

#define GAME_SCREEN_W 320
#define GAME_SCREEN_H 256
#define GAME_BPP 6
#define GAME_COLOR_COUNT (1 << GAME_BPP)
#define GAME_AGA_FMODE 3
#define HUD_H 16
#define GAME_VIEW_H (GAME_SCREEN_H - HUD_H)

extern tState gameState;

void gamePrepare(void);
void gameDiscardPrepared(void);

#endif /* GAME_H */
