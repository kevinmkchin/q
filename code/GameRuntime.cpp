#include "GameRuntime.h"
#include "Engine.h"
#include "Collision.h"
#include "Resources.h"
#include "GUI.h"
#include "LevelGeometry.h"
#include "GameRenderer.h"
#include "Input.h"


template <class T>
INTERNAL EntityIndex MakeSharedPtrAndPushBackToEntitiesVector(GameState* gameState)
{
	std::shared_ptr<Entity> ptr = std::make_shared<T>();
	EntityIndex id = (EntityIndex)gameState->entities.size();
	ptr->self = id;
	gameState->entities.push_back(ptr);
	return id;
}

class Player : public Entity
{
public:
	float yVel = 0.f;

private:
	virtual void Update()
	{
		float moveSpeed = 100.f;
		vec3 dir = Engine.gfx->activeGameCamera->calculatedDirection;
		dir.y = 0.f;
		dir = Normalize(dir);
		if (Input.IsKeyPressed(SDL_SCANCODE_W))
			transform.position += dir * moveSpeed * Time.deltaTime;
		if (Input.IsKeyPressed(SDL_SCANCODE_A))
			transform.position += -Engine.gfx->activeGameCamera->calculatedRight * moveSpeed * Time.deltaTime;
		if (Input.IsKeyPressed(SDL_SCANCODE_S))
			transform.position += -dir * moveSpeed * Time.deltaTime;
		if (Input.IsKeyPressed(SDL_SCANCODE_D))
			transform.position += Engine.gfx->activeGameCamera->calculatedRight * moveSpeed * Time.deltaTime;

		if (Input.HasKeyBeenPressed(SDL_SCANCODE_SPACE))
		{
			yVel = 10.f;
		}
		yVel -= 9.81f * Time.deltaTime;
		transform.position.y += yVel * Time.deltaTime;
	}

	virtual void Render()
	{

	}
};

EntityIndex CreateNewPlayer(GameState* gameState)
{
	EntityIndex id = MakeSharedPtrAndPushBackToEntitiesVector<Player>(gameState);
	gameState->player = id;
	return id;
}

//EntityIndex CreateNewMonster(GameState* gameState)
//{
//	EntityIndex id = MakeSharedPtrAndPushBackToEntitiesVector<Monster>(gameState);
//	gameState->monsterIds.push_back(id);
//	return id;
//}

void GameRuntime::LoadAllContent()
{
	MeshGroup::MeshGroupAssimpLoad(primitives_path("cube.obj").c_str());

	LevelGeometry::CreateFaceBatchMeshOnGPU(&state.faces_0.idVAO, &state.faces_0.idVBO);
}

void GameRuntime::UnloadAllContent()
{
	LevelGeometry::DeleteFaceBatchMeshOnGPU(state.faces_0.idVAO, state.faces_0.idVBO);
}

// temporary
void bruh(CollisionEventArgs args)
{
	if (args.other == 451)
	{
		std::shared_ptr<Entity> entity = Engine.game->state.entities[Engine.game->state.player];
		std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(entity);
		if (Dot(Normalize(args.normal), GM_UP_VECTOR) > 0.5f)
		{
			player->transform.position += GM_UP_VECTOR * args.depth;
			player->yVel = 0.f;
		}
		else if (Dot(Normalize(args.normal), GM_UP_VECTOR) < -0.5f)
		{
			player->yVel = 0.f;
			player->transform.position += args.normal * args.depth;
		}
		else
		{
			player->transform.position += args.normal * args.depth;
		}
	}
}
EntityTransform tra;
MeshCollider col0 = MeshCollider(451, &tra, bruh);
MeshCollider col1 = MeshCollider(451, &tra, bruh);


void GameRuntime::StartGame()
{
	state.playerCamera.position = vec3(-200.f, 0.f, 0.f);
	Engine.gfx->activeGameCamera = &state.playerCamera;

	float faces_data[] = {
		128, -32, 128, 1, 1, 0, 0, 0,
		-128, -32, 128, 0, 1, 0, 0, 0,
		-128, -32, -128, 0, 0, 0, 0, 0,

		-128, -32, -128, 0, 0, 0, 0, 0,
		128, -32, -128, 1, 0, 0, 0, 0,
		128, -32, 128, 1, 1, 0, 0, 0,
	};

	LevelGeometry::RebindFaceBatchBufferObject(&state.faces_0, sizeof(faces_data), faces_data);
	//faces_0.facesTexture = Resources.defaultMissingTexture;

	tra = EntityTransform();
	col0.verticesModelSpace = { 
		vec3(128, -32, 128),
		vec3(-128, -32, 128),
		vec3(-128, -32, -128),
		vec3(128, -32, -128),

		vec3(128, -40, 128),
		vec3(-128, -40, 128),
		vec3(-128, -40, -128),
		vec3(128, -40, -128),
	};
	CreateNewPlayer(&state);
	std::shared_ptr<Entity> player = state.entities[state.player];
	col1 = MeshCollider(player->self, &(player->transform), bruh);
	col1.verticesModelSpace = {
			{-16.f, 16.f, -16.f},
			{-16.f, 16.f, 16.f},
			{16.f, 16.f, 16.f},
			{16.f, 16.f, -16.f},
			{-16.f, -16.f, -16.f},
			{-16.f, -16.f, 16.f},
			{16.f, -16.f, 16.f},
			{16.f, -16.f, -16.f}
	};

	//std::shared_ptr<RotatingCube> p = std::make_shared<RotatingCube>();
	//EntityIndex id = (EntityIndex)gameState->entities.size();
	//p->self = id;
	//gameState->entities.push_back(p);
	//p->graphics.sharedMesh = MeshGroup::MeshGroupAssimpLoad(primitives_path("cube.obj").c_str());
	//gameState->rotatingCube = id;


}

void GameRuntime::Update()
{
	KevGui::DoTextUnformatted("GAME", 25, 40, 16, KevGui::TextAlignment::Left);
	KevGui::DoPanel(KevGui::UIRect(22, 45, 140, 22), vec4(1.f, 0.f, 0.f, 1.f));
	KevGui::DoText(25, 64, 16, KevGui::TextAlignment::Left, "Health: 99");

	//float moveSpeed = 100.f;
	//vec3 cameraPosition = state.playerCamera.position;
	//if (Input.IsKeyPressed(SDL_SCANCODE_W))
	//	cameraPosition += state.playerCamera.calculatedDirection * moveSpeed * Time.deltaTime;
	//if (Input.IsKeyPressed(SDL_SCANCODE_A))
	//	cameraPosition += -state.playerCamera.calculatedRight * moveSpeed * Time.deltaTime;
	//if (Input.IsKeyPressed(SDL_SCANCODE_S))
	//	cameraPosition += -state.playerCamera.calculatedDirection * moveSpeed * Time.deltaTime;
	//if (Input.IsKeyPressed(SDL_SCANCODE_D))
	//	cameraPosition += state.playerCamera.calculatedRight * moveSpeed * Time.deltaTime;
	//if (Input.IsKeyPressed(SDL_SCANCODE_Q))
	//	cameraPosition.y += -moveSpeed * Time.deltaTime;
	//if (Input.IsKeyPressed(SDL_SCANCODE_E))
	//	cameraPosition.y += moveSpeed * Time.deltaTime;

	SDL_SetRelativeMouseMode(SDL_TRUE);
	std::shared_ptr<Entity> player = state.entities[state.player];
	state.playerCamera.CameraUpdate(player->transform.position, Input.mouseDelta.x * 1.7f, Input.mouseDelta.y * 1.7f);
	state.playerCamera.CameraCalculatePerspectiveMatrix();

	CheckCollisionAndNotify(&col0, &col1);

	GameState* gameState = &state;

	for (auto e : gameState->entities)
	{
		e->Update();
	}

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
