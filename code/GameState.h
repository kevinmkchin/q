#pragma once

#include "Common.h"
#include "GameComponents.h"
#include "GameEntities.h"

struct GameState
{
    std::vector<std::shared_ptr<Entity>> entities;
    EntityIndex player;
    //std::vector<EntityIndex> playerIds;
    std::vector<EntityIndex> monsterIds;
    std::vector<EntityIndex> npcIds;

    EntityIndex rotatingCube;
};

EntityIndex CreateNewPlayer(GameState* gameState);

EntityIndex CreateNewMonster(GameState* gameState);


