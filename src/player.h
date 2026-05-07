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
/** 1 while dodge roll is active (for future combat / weapons). */
UBYTE playerIsInDodge(void);

#endif /* GAME_PLAYER_H */
