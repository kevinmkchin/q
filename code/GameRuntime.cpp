#include "GameRuntime.h"
#include "Engine.h"
#include "Collision.h"
#include "Resources.h"
#include "GUI.h"


template <class T>
INTERNAL EntityIndex MakeSharedPtrAndPushBackToEntitiesVector(GameState* gameState)
{
	std::shared_ptr<Entity> ptr = std::make_shared<T>();
	EntityIndex id = (EntityIndex)gameState->entities.size();
	ptr->self = id;
	gameState->entities.push_back(ptr);
	return id;
}

//EntityIndex CreateNewPlayer(GameState* gameState)
//{
//	EntityIndex id = MakeSharedPtrAndPushBackToEntitiesVector<Player>(gameState);
//	gameState->player = id;
//	return id;
//}
//
//EntityIndex CreateNewMonster(GameState* gameState)
//{
//	EntityIndex id = MakeSharedPtrAndPushBackToEntitiesVector<Monster>(gameState);
//	gameState->monsterIds.push_back(id);
//	return id;
//}


void GameRuntime::LoadAllContent()
{
	MeshGroup::MeshGroupAssimpLoad(primitives_path("cube.obj").c_str());
}

void GameRuntime::UnloadAllContent()
{

}

void GameRuntime::StartGame()
{
	GameState* gameState = &state;

	//std::shared_ptr<RotatingCube> p = std::make_shared<RotatingCube>();
	//EntityIndex id = (EntityIndex)gameState->entities.size();
	//p->self = id;
	//gameState->entities.push_back(p);
	//p->graphics.sharedMesh = MeshGroup::MeshGroupAssimpLoad(primitives_path("cube.obj").c_str());
	//gameState->rotatingCube = id;
}

void GameRuntime::Update()
{
	KevGui::DoText("GAME", 25, 40, 16);

	GameState* gameState = &state;

	for (auto e : gameState->entities)
	{
		e->Update();
	}

	//CheckCollisionAndNotify(&A->collier, &B->collider);

//    if (true) // PAUSED or console_is_hidden()
//    {
//        // Screen PrePhysicsStep
//        // Entities PrePhysicsStep
//        // Physics
//        // Handle Collision Events?
//        // Update Subsystems
//        // Screen Step
//        // Entities Step
//        ui.Step(Time.deltaTime);
//    }
}
