#include "game.h"

#include "aim.h"
#include "campaign.h"
#include "hud.h"
#include "scroller.h"

#include <ace/managers/key.h>
#include <ace/managers/system.h>
#include <ace/managers/state.h>
#include <ace/utils/extview.h>

extern tStateManager *stateMgr;

#define CAM_SPEED 4

static tView *view;
static tVPort *gameVport;

static void gameCreate(void) {
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

    viewLoad(view);
    systemUnuse();
}

static void gameLoop(void) {
    WORD dx = 0, dy = 0;
    if (keyCheck(KEY_LEFT)  || keyCheck(KEY_A)) { dx = -CAM_SPEED; }
    if (keyCheck(KEY_RIGHT) || keyCheck(KEY_D)) { dx =  CAM_SPEED; }
    if (keyCheck(KEY_UP)    || keyCheck(KEY_W)) { dy = -CAM_SPEED; }
    if (keyCheck(KEY_DOWN)  || keyCheck(KEY_S)) { dy =  CAM_SPEED; }

    if (keyUse(KEY_ESCAPE)) {
        campaignSetLevelResult(LEVEL_RESULT_EXIT_TO_TITLE);
        stateChange(stateMgr, &campaignState);
        return;
    }

    if (keyUse(KEY_RETURN) || keyUse(KEY_SPACE)) {
        campaignSetLevelResult(LEVEL_RESULT_COMPLETED);
        stateChange(stateMgr, &campaignState);
        return;
    }

    scrollerMoveCamera(dx, dy);
    aimProcess();
    scrollerProcess();
    viewProcessManagers(view);
    copProcessBlocks();
    vPortWaitForEnd(gameVport);
}

static void gameDestroy(void) {
    systemUse();
    viewLoad(0);
    aimDestroy();
    viewDestroy(view);
    scrollerDestroy();
    hudDestroy();
    view = 0;
    gameVport = 0;
}

tState gameState = {
    .cbCreate  = gameCreate,
    .cbLoop    = gameLoop,
    .cbDestroy = gameDestroy,
    .cbSuspend = 0,
    .cbResume  = 0,
};
