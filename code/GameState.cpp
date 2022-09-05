#include "GameState.h"

template <class T>
INTERNAL EntityIndex MakeSharedPtrAndPushBackToEntitiesVector(GameState* gameState)
{
	std::shared_ptr<Entity> ptr = std::make_shared<T>();
	EntityIndex id = (EntityIndex) gameState->entities.size();
	ptr->self = id;
	gameState->entities.push_back(ptr);
	return id;
}

EntityIndex CreateNewPlayer(GameState* gameState)
{
	EntityIndex id = MakeSharedPtrAndPushBackToEntitiesVector<Player>(gameState);
	gameState->player = id;
	return id;
}

EntityIndex CreateNewMonster(GameState* gameState)
{
	EntityIndex id = MakeSharedPtrAndPushBackToEntitiesVector<Monster>(gameState);
	gameState->monsterIds.push_back(id);
	return id;
}
