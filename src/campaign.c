#include "campaign.h"

#include "game.h"
#include "get_ready.h"
#include "title.h"

#include <ace/managers/state.h>

extern tStateManager *stateMgr;

static const LevelDefinition boulevardLevels[] = {
    {
        "BULWAR 1",
        OBJECTIVE_KILL_ALL,
        0,
        "data/bitmaps/get_ready.bm",
        "data/palettes/get_ready.plt"
    },
    {
        "BULWAR 2",
        OBJECTIVE_COLLECT_ITEM,
        1,
        "data/bitmaps/get_ready.bm",
        "data/palettes/get_ready.plt"
    },
};

static const WorldDefinition worlds[] = {
    {
        "BULWAR",
        boulevardLevels,
        sizeof(boulevardLevels) / sizeof(boulevardLevels[0])
    },
};

CampaignSession campaignSession;

static void campaignAdvanceLevel(void) {
    const WorldDefinition *world = campaignGetCurrentWorld();

    ++campaignSession.levelIndex;
    if (campaignSession.levelIndex < world->levelCount) {
        return;
    }

    campaignSession.levelIndex = 0;
    ++campaignSession.worldIndex;
    if (campaignSession.worldIndex >= (sizeof(worlds) / sizeof(worlds[0]))) {
        campaignSession.worldIndex = 0;
    }
}

void campaignStartNewGame(void) {
    campaignSession.worldIndex = 0;
    campaignSession.levelIndex = 0;
    campaignSession.lives = 3;
    campaignSession.money = 0;
    campaignSession.weaponLevel = 0;
    campaignSession.lastLevelResult = LEVEL_RESULT_NONE;
    campaignSession.route = CAMPAIGN_ROUTE_GET_READY;
}

void campaignSetRoute(CampaignRoute route) {
    campaignSession.route = route;
}

void campaignSetLevelResult(LevelResult result) {
    campaignSession.lastLevelResult = result;
    campaignSession.route = CAMPAIGN_ROUTE_NONE;
}

const WorldDefinition *campaignGetCurrentWorld(void) {
    return &worlds[campaignSession.worldIndex];
}

const LevelDefinition *campaignGetCurrentLevel(void) {
    const WorldDefinition *world = campaignGetCurrentWorld();
    return &world->levels[campaignSession.levelIndex];
}

static void campaignCreate(void) {
}

static void campaignLoop(void) {
    if (campaignSession.route == CAMPAIGN_ROUTE_NONE) {
        switch (campaignSession.lastLevelResult) {
            case LEVEL_RESULT_COMPLETED:
                campaignSession.money += 100;
                campaignAdvanceLevel();
                campaignSession.route = CAMPAIGN_ROUTE_GET_READY;
                break;
            case LEVEL_RESULT_PLAYER_DEAD:
                if (campaignSession.lives) {
                    --campaignSession.lives;
                }
                campaignSession.route = campaignSession.lives ? CAMPAIGN_ROUTE_GET_READY : CAMPAIGN_ROUTE_GAME_OVER;
                break;
            case LEVEL_RESULT_EXIT_TO_TITLE:
                campaignSession.route = CAMPAIGN_ROUTE_TITLE;
                break;
            case LEVEL_RESULT_NONE:
            default:
                campaignSession.route = CAMPAIGN_ROUTE_GET_READY;
                break;
        }

        campaignSession.lastLevelResult = LEVEL_RESULT_NONE;
    }

    switch (campaignSession.route) {
        case CAMPAIGN_ROUTE_GET_READY:
            stateChange(stateMgr, &getReadyState);
            break;
        case CAMPAIGN_ROUTE_GAMEPLAY:
            stateChange(stateMgr, &gameState);
            break;
        case CAMPAIGN_ROUTE_TITLE:
        case CAMPAIGN_ROUTE_GAME_OVER:
            stateChange(stateMgr, &titleState);
            break;
        case CAMPAIGN_ROUTE_SHOP:
        case CAMPAIGN_ROUTE_NONE:
        default:
            campaignSession.route = CAMPAIGN_ROUTE_GET_READY;
            break;
    }
}

static void campaignDestroy(void) {
}

tState campaignState = {
    .cbCreate = campaignCreate,
    .cbLoop = campaignLoop,
    .cbDestroy = campaignDestroy,
    .cbSuspend = 0,
    .cbResume = 0,
};
