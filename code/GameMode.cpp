#include "GameMode.h"
#include "Engine.h"
#include "Collision.h"
#include "Resources.h"
#include "GameState.h"


void GameMode::LoadAllContent()
{
	MeshGroup::MeshGroupAssimpLoad(primitives_path("cube.obj").c_str());
}

void GameMode::UnloadAllContent()
{

}

void GameMode::StartGame()
{
	GameState* gameState = Engine.GetGameState();

	std::shared_ptr<RotatingCube> p = std::make_shared<RotatingCube>();
	EntityIndex id = (EntityIndex)gameState->entities.size();
	p->self = id;
	gameState->entities.push_back(p);
	p->graphics.sharedMesh = MeshGroup::MeshGroupAssimpLoad(primitives_path("cube.obj").c_str());
	gameState->rotatingCube = id;
}

void GameMode::Update()
{
	GameState* gameState = Engine.GetGameState();

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
