#include "GameMode.h"
#include "Engine.h"
#include "Collision.h"

void GameMode::LoadAllContent()
{

}

void GameMode::UnloadAllContent()
{

}

void GameMode::StartGame()
{
	CreateNewPlayer(Engine.GetGameState());
}

void GameMode::Update()
{
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
