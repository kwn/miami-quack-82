#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include <ace/types.h>

void playerCreate(void);
void playerProcess(void);
void playerRender(void);
void playerDestroy(void);
UWORD playerGetX(void);
UWORD playerGetY(void);
UBYTE playerHasMovementInput(void);

#endif /* GAME_PLAYER_H */
