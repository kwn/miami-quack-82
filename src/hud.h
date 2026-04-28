#ifndef GAME_HUD_H
#define GAME_HUD_H

#include "game.h"

#include <ace/utils/extview.h>

tVPort *hudCreate(tView *view);
void hudDestroy(void);

#endif /* GAME_HUD_H */
