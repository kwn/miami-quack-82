#include "bullet.h"

#include "game.h"
#include "math_utils.h"
#include "scroller.h"

#include <ace/managers/bob.h>
#include <ace/utils/bitmap.h>

#define BULLET_MAX_COUNT 48
#define BULLET_W 16
#define BULLET_H 16
#define BULLET_FP_SHIFT 4

typedef struct tBulletGfx {
	tBitMap *pBitmap;
	tBitMap *pMask;
	UBYTE *pFrame;
	UBYTE *pMaskFrame;
	UBYTE isLoaded;
} tBulletGfx;

typedef struct tBullet {
	tBob sBob;
	LONG lXfp;
	LONG lYfp;
	WORD wVxFp;
	WORD wVyFp;
	UWORD uwDistance;
	UWORD uwMaxDistance;
	tBulletOwner eOwner;
	tBulletType eType;
	UBYTE isActive;
} tBullet;

static const char *s_pBulletBitmapPaths[BULLET_TYPE_COUNT] = {
	"data/bullets/bullet_small.bm",
	"data/bullets/bullet_flame.bm",
};

static const char *s_pBulletMaskPaths[BULLET_TYPE_COUNT] = {
	"data/bullets/bullet_small_mask.bm",
	"data/bullets/bullet_flame_mask.bm",
};

static tBulletGfx s_pBulletGfx[BULLET_TYPE_COUNT];
static tBullet s_pBullets[BULLET_MAX_COUNT];
static UBYTE s_isBobReady;

static UBYTE loadBulletGfx(tBulletType eType) {
	tBulletGfx *pGfx = &s_pBulletGfx[eType];

	pGfx->pBitmap = bitmapCreateFromPath(s_pBulletBitmapPaths[eType], 0);
	pGfx->pMask = bitmapCreateFromPath(s_pBulletMaskPaths[eType], 0);
	if(!pGfx->pBitmap || !pGfx->pMask) {
		return 0;
	}

	pGfx->pFrame = bobCalcFrameAddress(pGfx->pBitmap, 0);
	pGfx->pMaskFrame = bobCalcFrameAddress(pGfx->pMask, 0);
	pGfx->isLoaded = 1;
	return 1;
}

static void destroyBulletGfx(tBulletGfx *pGfx) {
	if(pGfx->pBitmap) {
		bitmapDestroy(pGfx->pBitmap);
	}
	if(pGfx->pMask) {
		bitmapDestroy(pGfx->pMask);
	}
	*pGfx = (tBulletGfx){0};
}

static UBYTE findInitialBulletType(void) {
	for(UBYTE ubType = 0; ubType < BULLET_TYPE_COUNT; ++ubType) {
		if(s_pBulletGfx[ubType].isLoaded) {
			return ubType;
		}
	}
	return BULLET_TYPE_COUNT;
}

static UBYTE isOutsideWorld(WORD wX, WORD wY) {
	return (
		wX < -BULLET_W ||
		wY < -BULLET_H ||
		wX > (WORD)scrollerGetWorldWidth() ||
		wY > (WORD)scrollerGetWorldHeight()
	);
}

void bulletCreate(void) {
	for(UBYTE ubType = 0; ubType < BULLET_TYPE_COUNT; ++ubType) {
		loadBulletGfx((tBulletType)ubType);
	}

	UBYTE ubInitialType = findInitialBulletType();
	if(ubInitialType >= BULLET_TYPE_COUNT) {
		s_isBobReady = 0;
		return;
	}

	for(UBYTE ubBullet = 0; ubBullet < BULLET_MAX_COUNT; ++ubBullet) {
		tBullet *pBullet = &s_pBullets[ubBullet];
		pBullet->isActive = 0;
		bobInit(
			&pBullet->sBob,
			BULLET_W,
			BULLET_H,
			1,
			s_pBulletGfx[ubInitialType].pFrame,
			s_pBulletGfx[ubInitialType].pMaskFrame,
			0,
			0
		);
	}
	s_isBobReady = 1;
}

void bulletProcess(void) {
	for(UBYTE ubBullet = 0; ubBullet < BULLET_MAX_COUNT; ++ubBullet) {
		tBullet *pBullet = &s_pBullets[ubBullet];
		if(!pBullet->isActive) {
			continue;
		}

		pBullet->lXfp += pBullet->wVxFp;
		pBullet->lYfp += pBullet->wVyFp;
		pBullet->uwDistance += (UWORD)mathApproxVectorLength(
			pBullet->wVxFp >> BULLET_FP_SHIFT,
			pBullet->wVyFp >> BULLET_FP_SHIFT
		);

		WORD wX = (WORD)(pBullet->lXfp >> BULLET_FP_SHIFT);
		WORD wY = (WORD)(pBullet->lYfp >> BULLET_FP_SHIFT);
		if(pBullet->uwDistance >= pBullet->uwMaxDistance || isOutsideWorld(wX, wY)) {
			pBullet->isActive = 0;
		}
	}
}

void bulletRender(void) {
	if(!s_isBobReady) {
		return;
	}

	for(UBYTE ubBullet = 0; ubBullet < BULLET_MAX_COUNT; ++ubBullet) {
		tBullet *pBullet = &s_pBullets[ubBullet];
		if(!pBullet->isActive) {
			continue;
		}

		pBullet->sBob.sPos.uwX = (UWORD)(pBullet->lXfp >> BULLET_FP_SHIFT);
		pBullet->sBob.sPos.uwY = (UWORD)(pBullet->lYfp >> BULLET_FP_SHIFT);
		bobPush(&pBullet->sBob);
	}
}

void bulletDestroy(void) {
	for(UBYTE ubBullet = 0; ubBullet < BULLET_MAX_COUNT; ++ubBullet) {
		s_pBullets[ubBullet].isActive = 0;
	}
	for(UBYTE ubType = 0; ubType < BULLET_TYPE_COUNT; ++ubType) {
		destroyBulletGfx(&s_pBulletGfx[ubType]);
	}
	s_isBobReady = 0;
}

void bulletSpawn(
	tBulletOwner eOwner,
	tBulletType eType,
	WORD wStartX,
	WORD wStartY,
	WORD wDx,
	WORD wDy,
	UWORD uwSpeed,
	UWORD uwMaxDistance
) {
	if(!s_isBobReady || eType >= BULLET_TYPE_COUNT || !s_pBulletGfx[eType].isLoaded) {
		return;
	}

	int iLen = mathApproxVectorLength(wDx, wDy);
	if(iLen <= 0) {
		return;
	}

	for(UBYTE ubBullet = 0; ubBullet < BULLET_MAX_COUNT; ++ubBullet) {
		tBullet *pBullet = &s_pBullets[ubBullet];
		if(pBullet->isActive) {
			continue;
		}

		pBullet->lXfp = ((LONG)wStartX) << BULLET_FP_SHIFT;
		pBullet->lYfp = ((LONG)wStartY) << BULLET_FP_SHIFT;
		pBullet->wVxFp = (WORD)((((LONG)wDx * uwSpeed) << BULLET_FP_SHIFT) / iLen);
		pBullet->wVyFp = (WORD)((((LONG)wDy * uwSpeed) << BULLET_FP_SHIFT) / iLen);
		pBullet->uwDistance = 0;
		pBullet->uwMaxDistance = uwMaxDistance;
		pBullet->eOwner = eOwner;
		pBullet->eType = eType;
		pBullet->isActive = 1;
		bobSetFrame(
			&pBullet->sBob,
			s_pBulletGfx[eType].pFrame,
			s_pBulletGfx[eType].pMaskFrame
		);
		return;
	}
}
