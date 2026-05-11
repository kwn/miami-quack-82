#include "weapon.h"

#include "bullet.h"
#include "game.h"
#include "math_utils.h"
#include "scroller.h"

#include <ace/managers/bob.h>
#include <ace/managers/key.h>
#include <ace/managers/mouse.h>
#include <ace/utils/bitmap.h>

#define WEAPON_FRAME_COUNT 9
#define WEAPON_FRAME_W 16
#define WEAPON_FRAME_H 16
#define WEAPON_SHEET_H (WEAPON_FRAME_COUNT * WEAPON_FRAME_H)
#define WEAPON_ORBIT_RADIUS 18
#define PLAYER_HALF_W 8
#define PLAYER_HALF_H 8
#define WEAPON_FLAMETHROWER_TYPE 6
#define WEAPON_MOUSE_PORT MOUSE_PORT_1
#define WEAPON_SWITCH_COOLDOWN_FRAMES 10

typedef struct tWeaponGfx {
	tBitMap *pRight;
	tBitMap *pRightMask;
	tBitMap *pLeft;
	tBitMap *pLeftMask;
	UBYTE *pRightFrames[WEAPON_FRAME_COUNT];
	UBYTE *pRightMasks[WEAPON_FRAME_COUNT];
	UBYTE *pLeftFrames[WEAPON_FRAME_COUNT];
	UBYTE *pLeftMasks[WEAPON_FRAME_COUNT];
	UBYTE isLoaded;
} tWeaponGfx;

typedef struct tWeaponStats {
	UWORD uwCooldownFrames;
	UWORD uwBulletSpeed;
	UWORD uwBulletMaxDistance;
	UWORD uwDamage;
	UWORD uwKnockback;
	UWORD uwDeathKnockback;
	WORD wClipSize;
	WORD wMaxTotalAmmo;
	UWORD uwReloadFrames;
	UWORD uwNoiseRadius;
	UBYTE ubBulletCount;
	UBYTE ubSpread;
} tWeaponStats;

// cooldown, speed, distance, damage, knockback, death_knockback,
// clip_size, max_ammo, reload_frames, noise_radius, bullet_count, spread
static const tWeaponStats s_pWeaponStats[WEAPON_TYPE_COUNT] = {
	{ 10,  6,  20,  40, 2, 3,  -1,  -1,   0,   0, 1, 0 }, // knife
	{ 20, 16, 150,  30, 4, 5,  10,  50,  60, 250, 1, 0 }, // pistol
	{ 60, 10, 100, 100, 8, 10,  2,  20,  80, 400, 3, 3 }, // shotgun
	{100, 24, 400, 100, 6, 8,   4,  20, 100, 500, 1, 0 }, // sniper
	{  8, 18, 110,  20, 3, 4,  30, 150,  70, 200, 1, 0 }, // uzi
	{ 14, 14, 160,  50, 4, 5,  20, 100,  80, 300, 1, 0 }, // m16
	{  6,  8, 100,  10, 4, 5,  50, 200, 120, 150, 2, 2 }, // flamethrower
	{  4, 16, 160,  20, 3, 4, 100, 300, 150, 350, 1, 0 }, // minigun
};

static const char *s_pWeaponBitmapPaths[WEAPON_TYPE_COUNT] = {
	"data/weapons/weapon_knife.bm",
	"data/weapons/weapon_gun.bm",
	"data/weapons/weapon_shotgun.bm",
	"data/weapons/weapon_sniper.bm",
	"data/weapons/weapon_uzi.bm",
	"data/weapons/weapon_m16.bm",
	"data/weapons/weapon_flamethrower.bm",
	"data/weapons/weapon_minigun.bm",
};

static const char *s_pWeaponMaskPaths[WEAPON_TYPE_COUNT] = {
	"data/weapons/weapon_knife_mask.bm",
	"data/weapons/weapon_gun_mask.bm",
	"data/weapons/weapon_shotgun_mask.bm",
	"data/weapons/weapon_sniper_mask.bm",
	"data/weapons/weapon_uzi_mask.bm",
	"data/weapons/weapon_m16_mask.bm",
	"data/weapons/weapon_flamethrower_mask.bm",
	"data/weapons/weapon_minigun_mask.bm",
};

static const UBYTE s_pLeftFrameIndex[WEAPON_FRAME_COUNT] = {7, 6, 5, 4, 3, 2, 1, 0, 8};

static tWeaponGfx s_pWeaponGfx[WEAPON_TYPE_COUNT];
static tBob s_sWeaponBob;
static UBYTE s_ubCurrentWeapon;
static UBYTE s_isBobReady;
static UBYTE s_ubWeaponDir;
static WORD s_wWeaponOffsetX;
static WORD s_wWeaponOffsetY;
static UWORD s_uwWeaponX;
static UWORD s_uwWeaponY;
static UWORD s_uwShootCooldown;
static UWORD s_uwReloadTimer;
static WORD s_pAmmoInClip[WEAPON_TYPE_COUNT];
static WORD s_pTotalAmmo[WEAPON_TYPE_COUNT];

static UBYTE reverseByte(UBYTE ubValue) {
	ubValue = ((ubValue & 0xF0) >> 4) | ((ubValue & 0x0F) << 4);
	ubValue = ((ubValue & 0xCC) >> 2) | ((ubValue & 0x33) << 2);
	return ((ubValue & 0xAA) >> 1) | ((ubValue & 0x55) << 1);
}

static void flipBitmap(const tBitMap *pSrc, tBitMap *pDst) {
	UWORD uwSrcByteWidth = bitmapGetByteWidth(pSrc);
	UWORD uwDstByteWidth = bitmapGetByteWidth(pDst);

	for(UWORD uwY = 0; uwY < WEAPON_SHEET_H; ++uwY) {
		for(UBYTE ubPlane = 0; ubPlane < pSrc->Depth; ++ubPlane) {
			UBYTE *pSrcRow = (UBYTE *)pSrc->Planes[ubPlane] + (uwY * pSrc->BytesPerRow);
			UBYTE *pDstRow = (UBYTE *)pDst->Planes[ubPlane] + (uwY * pDst->BytesPerRow);

			for(UWORD uwByte = 0; uwByte < uwDstByteWidth; ++uwByte) {
				pDstRow[uwByte] = reverseByte(pSrcRow[uwSrcByteWidth - 1 - uwByte]);
			}
		}
	}
}

static void buildFramePointers(tWeaponGfx *pGfx) {
	for(UWORD uwFrame = 0; uwFrame < WEAPON_FRAME_COUNT; ++uwFrame) {
		UWORD uwOffsetY = uwFrame * WEAPON_FRAME_H;
		pGfx->pRightFrames[uwFrame] = bobCalcFrameAddress(pGfx->pRight, uwOffsetY);
		pGfx->pRightMasks[uwFrame] = bobCalcFrameAddress(pGfx->pRightMask, uwOffsetY);
		pGfx->pLeftFrames[uwFrame] = bobCalcFrameAddress(pGfx->pLeft, uwOffsetY);
		pGfx->pLeftMasks[uwFrame] = bobCalcFrameAddress(pGfx->pLeftMask, uwOffsetY);
	}
}

static UBYTE loadWeaponGfx(UBYTE ubWeaponType) {
	tWeaponGfx *pGfx = &s_pWeaponGfx[ubWeaponType];

	pGfx->pRight = bitmapCreateFromPath(s_pWeaponBitmapPaths[ubWeaponType], 0);
	pGfx->pRightMask = bitmapCreateFromPath(s_pWeaponMaskPaths[ubWeaponType], 0);
	if(!pGfx->pRight || !pGfx->pRightMask) {
		return 0;
	}

	pGfx->pLeft = bitmapCreate(WEAPON_FRAME_W, WEAPON_SHEET_H, pGfx->pRight->Depth, BMF_CLEAR | BMF_INTERLEAVED);
	pGfx->pLeftMask = bitmapCreate(WEAPON_FRAME_W, WEAPON_SHEET_H, pGfx->pRightMask->Depth, BMF_CLEAR | BMF_INTERLEAVED);
	if(!pGfx->pLeft || !pGfx->pLeftMask) {
		return 0;
	}

	flipBitmap(pGfx->pRight, pGfx->pLeft);
	flipBitmap(pGfx->pRightMask, pGfx->pLeftMask);
	buildFramePointers(pGfx);
	pGfx->isLoaded = 1;
	return 1;
}

static void destroyWeaponGfx(tWeaponGfx *pGfx) {
	if(pGfx->pRight) {
		bitmapDestroy(pGfx->pRight);
	}
	if(pGfx->pRightMask) {
		bitmapDestroy(pGfx->pRightMask);
	}
	if(pGfx->pLeft) {
		bitmapDestroy(pGfx->pLeft);
	}
	if(pGfx->pLeftMask) {
		bitmapDestroy(pGfx->pLeftMask);
	}
	*pGfx = (tWeaponGfx){0};
}

static void initWeaponAmmo(void) {
	for(UBYTE ubWeapon = 0; ubWeapon < WEAPON_TYPE_COUNT; ++ubWeapon) {
		WORD wClipSize = s_pWeaponStats[ubWeapon].wClipSize;
		if(wClipSize == -1) {
			s_pAmmoInClip[ubWeapon] = -1;
			s_pTotalAmmo[ubWeapon] = -1;
		}
		else {
			s_pAmmoInClip[ubWeapon] = wClipSize;
			s_pTotalAmmo[ubWeapon] = wClipSize * 3;
		}
	}
}

static void processReload(void) {
	if(!s_uwReloadTimer) {
		return;
	}

	--s_uwReloadTimer;
	if(s_uwReloadTimer) {
		return;
	}

	const tWeaponStats *pStats = &s_pWeaponStats[s_ubCurrentWeapon];
	if(pStats->wClipSize == -1) {
		return;
	}

	WORD wNeeded = pStats->wClipSize - s_pAmmoInClip[s_ubCurrentWeapon];
	if(wNeeded <= 0) {
		return;
	}
	if(s_pTotalAmmo[s_ubCurrentWeapon] >= wNeeded) {
		s_pAmmoInClip[s_ubCurrentWeapon] += wNeeded;
		s_pTotalAmmo[s_ubCurrentWeapon] -= wNeeded;
	}
	else {
		s_pAmmoInClip[s_ubCurrentWeapon] += s_pTotalAmmo[s_ubCurrentWeapon];
		s_pTotalAmmo[s_ubCurrentWeapon] = 0;
	}
}

static void tryStartReload(void) {
	const tWeaponStats *pStats = &s_pWeaponStats[s_ubCurrentWeapon];
	if(s_uwReloadTimer || pStats->wClipSize == -1) {
		return;
	}
	if(s_pAmmoInClip[s_ubCurrentWeapon] >= pStats->wClipSize || s_pTotalAmmo[s_ubCurrentWeapon] <= 0) {
		return;
	}
	s_uwReloadTimer = pStats->uwReloadFrames;
}

static void spawnBullet(WORD wDx, WORD wDy) {
	if(s_uwShootCooldown || s_uwReloadTimer) {
		return;
	}

	const tWeaponStats *pStats = &s_pWeaponStats[s_ubCurrentWeapon];
	if(pStats->wClipSize != -1) {
		if(s_pAmmoInClip[s_ubCurrentWeapon] <= 0) {
			return;
		}
		--s_pAmmoInClip[s_ubCurrentWeapon];
	}

	tBulletType eType = s_ubCurrentWeapon == WEAPON_FLAMETHROWER_TYPE ? BULLET_TYPE_FLAME : BULLET_TYPE_SMALL;
	WORD wStartX = (WORD)s_uwWeaponX;
	WORD wStartY = (WORD)s_uwWeaponY;
	UBYTE ubBulletCount = pStats->ubBulletCount ? pStats->ubBulletCount : 1;

	for(UBYTE ubBullet = 0; ubBullet < ubBulletCount; ++ubBullet) {
		WORD wOffset = (WORD)(2 * ubBullet) - (WORD)(ubBulletCount - 1);
		WORD wSpreadDx = wDx + ((-wDy * wOffset * pStats->ubSpread) >> 4);
		WORD wSpreadDy = wDy + (( wDx * wOffset * pStats->ubSpread) >> 4);

		bulletSpawn(
			BULLET_OWNER_PLAYER,
			eType,
			wStartX,
			wStartY,
			wSpreadDx,
			wSpreadDy,
			pStats->uwBulletSpeed,
			pStats->uwBulletMaxDistance
		);
	}

	s_uwShootCooldown = pStats->uwCooldownFrames;
}

static UBYTE findFirstLoadedWeapon(void) {
	for(UBYTE ubWeapon = 0; ubWeapon < WEAPON_TYPE_COUNT; ++ubWeapon) {
		if(s_pWeaponGfx[ubWeapon].isLoaded) {
			return ubWeapon;
		}
	}
	return WEAPON_TYPE_COUNT;
}

static void getWeaponFrame(
	UBYTE ubWeaponType, UBYTE ubDir, UBYTE **pFrame, UBYTE **pMask
) {
	tWeaponGfx *pGfx = &s_pWeaponGfx[ubWeaponType];

	if(ubDir <= 8) {
		*pFrame = pGfx->pRightFrames[ubDir];
		*pMask = pGfx->pRightMasks[ubDir];
	}
	else {
		UBYTE ubLeftFrame = s_pLeftFrameIndex[ubDir - 9];
		*pFrame = pGfx->pLeftFrames[ubLeftFrame];
		*pMask = pGfx->pLeftMasks[ubLeftFrame];
	}
}

void weaponCreate(void) {
	for(UBYTE ubWeapon = 0; ubWeapon < WEAPON_TYPE_COUNT; ++ubWeapon) {
		loadWeaponGfx(ubWeapon);
	}

	s_ubCurrentWeapon = 1;
	if(!s_pWeaponGfx[s_ubCurrentWeapon].isLoaded) {
		s_ubCurrentWeapon = findFirstLoadedWeapon();
	}
	if(s_ubCurrentWeapon >= WEAPON_TYPE_COUNT) {
		s_isBobReady = 0;
		return;
	}

	UBYTE *pFrame;
	UBYTE *pMask;
	getWeaponFrame(s_ubCurrentWeapon, 0, &pFrame, &pMask);
	bobInit(&s_sWeaponBob, WEAPON_FRAME_W, WEAPON_FRAME_H, 1, pFrame, pMask, 0, 0);
	s_isBobReady = 1;
	s_uwShootCooldown = 0;
	s_uwReloadTimer = 0;
	initWeaponAmmo();
}

void weaponProcess(UWORD uwPlayerX, UWORD uwPlayerY, UWORD uwAimX, UWORD uwAimY) {
	if(!s_isBobReady) {
		return;
	}

	if(keyUse(KEY_1)) { weaponSetCurrent(0); }
	else if(keyUse(KEY_2)) { weaponSetCurrent(1); }
	else if(keyUse(KEY_3)) { weaponSetCurrent(2); }
	else if(keyUse(KEY_4)) { weaponSetCurrent(3); }
	else if(keyUse(KEY_5)) { weaponSetCurrent(4); }
	else if(keyUse(KEY_6)) { weaponSetCurrent(5); }
	else if(keyUse(KEY_7)) { weaponSetCurrent(6); }
	else if(keyUse(KEY_8)) { weaponSetCurrent(7); }

	processReload();
	if(keyUse(KEY_R) || mouseUse(WEAPON_MOUSE_PORT, MOUSE_RMB)) {
		tryStartReload();
	}

	WORD wPlayerViewX = (WORD)uwPlayerX - (WORD)scrollerGetCameraX() + PLAYER_HALF_W;
	WORD wPlayerViewY = (WORD)uwPlayerY - (WORD)scrollerGetCameraY() + PLAYER_HALF_H;
	WORD wDx = (WORD)uwAimX - wPlayerViewX;
	WORD wDy = (WORD)uwAimY - wPlayerViewY;
	int iLen = mathApproxVectorLength(wDx, wDy);

	if(iLen > 0) {
		s_ubWeaponDir = (UBYTE)mathDirection18FromVector(wDx, wDy);
		s_wWeaponOffsetX = (wDx * WEAPON_ORBIT_RADIUS) / iLen;
		s_wWeaponOffsetY = (wDy * WEAPON_ORBIT_RADIUS) / iLen;
	}

	UBYTE *pFrame;
	UBYTE *pMask;
	getWeaponFrame(s_ubCurrentWeapon, s_ubWeaponDir, &pFrame, &pMask);
	bobSetFrame(&s_sWeaponBob, pFrame, pMask);
	LONG lWeaponX = (LONG)uwPlayerX + s_wWeaponOffsetX;
	LONG lWeaponY = (LONG)uwPlayerY + s_wWeaponOffsetY;
	s_uwWeaponX = lWeaponX < 0 ? 0 : (UWORD)lWeaponX;
	s_uwWeaponY = lWeaponY < 0 ? 0 : (UWORD)lWeaponY;

	if(s_uwShootCooldown) {
		--s_uwShootCooldown;
	}
	if(mouseCheck(WEAPON_MOUSE_PORT, MOUSE_LMB)) {
		spawnBullet(wDx, wDy);
	}
}

void weaponRender(void) {
	if(!s_isBobReady) {
		return;
	}

	s_sWeaponBob.sPos.uwX = s_uwWeaponX;
	s_sWeaponBob.sPos.uwY = s_uwWeaponY;
	bobPush(&s_sWeaponBob);
}

void weaponDestroy(void) {
	for(UBYTE ubWeapon = 0; ubWeapon < WEAPON_TYPE_COUNT; ++ubWeapon) {
		destroyWeaponGfx(&s_pWeaponGfx[ubWeapon]);
	}
	s_isBobReady = 0;
}

void weaponSetCurrent(UBYTE ubWeaponType) {
	if(ubWeaponType >= WEAPON_TYPE_COUNT || !s_pWeaponGfx[ubWeaponType].isLoaded || ubWeaponType == s_ubCurrentWeapon) {
		return;
	}
	s_ubCurrentWeapon = ubWeaponType;
	s_uwReloadTimer = 0;
	s_uwShootCooldown = WEAPON_SWITCH_COOLDOWN_FRAMES;
}

UBYTE weaponGetCurrent(void) {
	return s_ubCurrentWeapon;
}
