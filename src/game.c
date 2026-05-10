#include "game.h"

#include "aim.h"
#include "campaign.h"
#include "game_camera.h"
#include "hud.h"
#include "player.h"
#include "scroller.h"
#include "weapon.h"

#include <ace/managers/bob.h>
#include <ace/managers/key.h>
#include <ace/managers/system.h>
#include <ace/managers/state.h>
#include <ace/utils/extview.h>

extern tStateManager *stateMgr;

static tView *view;
static tVPort *gameVport;
static UBYTE isPrepared;

void gamePrepare(void) {
    if (isPrepared) {
        return;
    }

    view = viewCreate(0,
        TAG_VIEW_GLOBAL_PALETTE, 1,
        TAG_VIEW_GLOBAL_BPP, 1,
#ifdef ACE_USE_AGA_FEATURES
        TAG_VIEW_USES_AGA, 1,
#endif
        TAG_DONE
    );

    hudCreate(view);
    gameVport = scrollerCreate(view);
    aimCreate(view);
    playerCreate();
    weaponCreate();
    bobReallocateBuffers();
    gameCameraCreate();
    gameCameraSetFocus(playerGetX(), playerGetY());
    gameCameraTrackPlayer(playerGetX(), playerGetY(), playerHasMovementInput());
    gameCameraSnapToFocus();
    scrollerRedrawAll();
    bobDiscardUndraw();
    isPrepared = 1;
}

static void gameCreate(void) {
    gamePrepare();
    viewLoad(view);
    systemUnuse();
}

static void gameLoop(void) {
    if (keyUse(KEY_ESCAPE)) {
        campaignSetLevelResult(LEVEL_RESULT_EXIT_TO_TITLE);
        stateChange(stateMgr, &campaignState);
        return;
    }

    if (keyUse(KEY_RETURN)) {
        campaignSetLevelResult(LEVEL_RESULT_COMPLETED);
        stateChange(stateMgr, &campaignState);
        return;
    }

    aimProcess();
    playerProcess();
    weaponProcess(playerGetX(), playerGetY(), aimGetX(), aimGetY());
    gameCameraTrackPlayer(playerGetX(), playerGetY(), playerHasMovementInput());
    gameCameraProcess();

    bobBegin(scrollerGetBackBuffer());
    playerRender();
    weaponRender();
    bobEnd();

    viewProcessManagers(view);
    copProcessBlocks();
    vPortWaitForEnd(gameVport);
}

void gameDiscardPrepared(void) {
    if (!isPrepared) {
        return;
    }

    gameCameraDestroy();
    weaponDestroy();
    playerDestroy();
    aimDestroy();
    viewDestroy(view);
    scrollerDestroy();
    hudDestroy();
    view = 0;
    gameVport = 0;
    isPrepared = 0;
}

static void gameDestroy(void) {
    systemUse();
    viewLoad(0);
    gameDiscardPrepared();
}

tState gameState = {
    .cbCreate  = gameCreate,
    .cbLoop    = gameLoop,
    .cbDestroy = gameDestroy,
    .cbSuspend = 0,
    .cbResume  = 0,
};
