#ifndef GAME_BULLET_H
#define GAME_BULLET_H

#include <ace/types.h>

typedef enum tBulletOwner {
	BULLET_OWNER_PLAYER,
	BULLET_OWNER_ENEMY,
} tBulletOwner;

typedef enum tBulletType {
	BULLET_TYPE_SMALL,
	BULLET_TYPE_FLAME,
	BULLET_TYPE_COUNT,
} tBulletType;

void bulletCreate(void);
void bulletProcess(void);
void bulletRender(void);
void bulletDestroy(void);
void bulletSpawn(
	tBulletOwner eOwner,
	tBulletType eType,
	WORD wStartX,
	WORD wStartY,
	WORD wDx,
	WORD wDy,
	UWORD uwSpeed,
	UWORD uwMaxDistance
);

#endif /* GAME_BULLET_H */
