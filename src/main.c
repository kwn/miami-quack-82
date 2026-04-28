/**
 * game – ACE boilerplate entrypoint
 *
 * The initial state is the title screen; campaign.c routes into gameState.
 */

#undef GENERIC_MAIN_LOG_PATH
#include <ace/generic/main.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include <ace/managers/state.h>
#include <ace/managers/game.h>

#include "title.h"

tStateManager *stateMgr;

void genericCreate(void) {
    joyOpen();
    keyCreate();
    stateMgr = stateManagerCreate();
    stateChange(stateMgr, &titleState);
}

void genericProcess(void) {
    joyProcess();
    keyProcess();
    stateProcess(stateMgr);
}

void genericDestroy(void) {
    stateManagerDestroy(stateMgr);
    keyDestroy();
    joyClose();
}
