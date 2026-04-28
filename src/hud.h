#ifndef GAME_HUD_H
#define GAME_HUD_H

#include <ace/types.h>
#include <ace/utils/extview.h>

#define GAME_SCREEN_W 320
#define GAME_SCREEN_H 256
#define GAME_BPP 5
#define GAME_COLOR_COUNT (1 << GAME_BPP)
#define HUD_H 16
#define GAME_VIEW_H (GAME_SCREEN_H - HUD_H)

tVPort *hudCreate(tView *view);
void hudDestroy(void);

#endif /* GAME_HUD_H */
