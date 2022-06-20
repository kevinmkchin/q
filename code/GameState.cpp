#include "GameState.h"

EntityIndex CreateNewPlayer(GameState* gameState)
{
	std::shared_ptr<Entity> p = std::make_shared<Player>();
	EntityIndex id = (EntityIndex) gameState->entities.size();
	p->self = id;
	gameState->entities.push_back(p);
	gameState->player = id;
	return id;
}

EntityIndex CreateNewMonster(GameState* gameState)
{
	std::shared_ptr<Entity> m = std::make_shared<Monster>();
	EntityIndex id = (EntityIndex) gameState->entities.size();
	m->self = id;
	gameState->entities.push_back(m);
	gameState->monsterIds.push_back(id);
	return id;
}
