/**
 * game – ACE boilerplate entrypoint
 *
 * To replace the scroller with your own game state:
 *   1. Delete scroller.c / scroller.h
 *   2. Create your own state file with gsMyGameCreate/Loop/Destroy
 *   3. Replace the stateChange() call below
 */

#undef GENERIC_MAIN_LOG_PATH          /* disable log file in release builds */
#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>
#include <ace/managers/game.h>

#include "scroller.h"

/* ---- global state manager (accessible from all game states) ------------ */
tStateManager *g_pStateMgr;

void genericCreate(void) {
    keyCreate();
    g_pStateMgr = stateManagerCreate();
    stateChange(g_pStateMgr, &g_sScrollerState);
}

void genericProcess(void) {
    keyProcess();
    stateProcess(g_pStateMgr);
}

void genericDestroy(void) {
    stateManagerDestroy(g_pStateMgr);
    keyDestroy();
}
