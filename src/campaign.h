#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include <ace/managers/state.h>
#include <ace/types.h>

typedef enum {
    OBJECTIVE_KILL_ALL,
    OBJECTIVE_COLLECT_ITEM,
    OBJECTIVE_KILL_BOSS,
    OBJECTIVE_SURVIVE_TIME
} ObjectiveKind;

typedef enum {
    CAMPAIGN_ROUTE_NONE,
    CAMPAIGN_ROUTE_GET_READY,
    CAMPAIGN_ROUTE_GAMEPLAY,
    CAMPAIGN_ROUTE_SHOP,
    CAMPAIGN_ROUTE_GAME_OVER,
    CAMPAIGN_ROUTE_TITLE
} CampaignRoute;

typedef enum {
    LEVEL_RESULT_NONE,
    LEVEL_RESULT_COMPLETED,
    LEVEL_RESULT_PLAYER_DEAD,
    LEVEL_RESULT_EXIT_TO_TITLE
} LevelResult;

typedef enum {
    WORLD_KIND_OUTDOOR,
    WORLD_KIND_LAKE
} WorldKind;

typedef struct {
    const char *name;
    ObjectiveKind objective;
    UWORD targetCount;
    const char *getReadyImagePath;
    const char *getReadyPalettePath;
} LevelDefinition;

typedef struct {
    const char *name;
    WorldKind kind;
    const char *tilesetPath;
    const LevelDefinition *levels;
    UBYTE levelCount;
} WorldDefinition;

typedef struct {
    UBYTE worldIndex;
    UBYTE levelIndex;
    UBYTE lives;
    UWORD money;
    UBYTE weaponLevel;
    CampaignRoute route;
    LevelResult lastLevelResult;
} CampaignSession;

extern CampaignSession campaignSession;
extern tState campaignState;

void campaignStartNewGame(void);
void campaignSetRoute(CampaignRoute route);
void campaignSetLevelResult(LevelResult result);
const WorldDefinition *campaignGetCurrentWorld(void);
const LevelDefinition *campaignGetCurrentLevel(void);

#endif /* CAMPAIGN_H */
