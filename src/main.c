/**
 * game – ACE boilerplate entrypoint
 *
 * To replace the scroller with your own game state:
 *   1. Delete scroller.c / scroller.h
 *   2. Create your own state file with the three callbacks
 *   3. Replace the stateChange() call below
 */

#undef GENERIC_MAIN_LOG_PATH
#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>
#include <ace/managers/game.h>

#include "scroller.h"

tStateManager *stateMgr;

void genericCreate(void) {
    keyCreate();
    stateMgr = stateManagerCreate();
    stateChange(stateMgr, &scrollerState);
}

void genericProcess(void) {
    keyProcess();
    stateProcess(stateMgr);
}

void genericDestroy(void) {
    stateManagerDestroy(stateMgr);
    keyDestroy();
}
