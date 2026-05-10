#ifndef GAME_WEAPON_H
#define GAME_WEAPON_H

#include <ace/types.h>

#define WEAPON_TYPE_COUNT 8

void weaponCreate(void);
void weaponProcess(UWORD playerX, UWORD playerY, UWORD aimX, UWORD aimY);
void weaponRender(void);
void weaponDestroy(void);
void weaponSetCurrent(UBYTE weaponType);
UBYTE weaponGetCurrent(void);

#endif /* GAME_WEAPON_H */
