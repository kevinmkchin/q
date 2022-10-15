#pragma once

#include <gmath.h>

#include "Common.h"
#include "LevelGeometry.h"
#include "ConstructiveSolidGeometry.h"
#include "Camera.h"

struct EntityTransform
{
	vec3 position;
	quat orientation;
	vec3 scale = { 1.f,1.f,1.f };

	mat4 GetModelMatrix()
	{
		mat4 modelMatrix = mat4();
		modelMatrix *= TranslationMatrix(position);
		modelMatrix *= RotationMatrix(orientation);
		modelMatrix *= ScaleMatrix(scale);
		return modelMatrix;
	}
};

class Entity
{
public:
	virtual void Update() = 0;
	virtual void Render() = 0;

public:
	EntityTransform transform;
	EntityIndex self = EntityIndexInvalid;
};

struct GameState
{
    std::vector<std::shared_ptr<Entity>> entities;
    EntityIndex player;
    std::vector<EntityIndex> monsterIds;
    std::vector<EntityIndex> npcIds;

    EntityIndex rotatingCube;

	GameCamera playerCamera;

};

//class Player : Entity
//{
//public:
//	virtual void Update() override
//	{
//
//	}
//
//	virtual void Render() override
//	{
//
//	}
//
//
//};

//EntityIndex CreateNewPlayer(GameState* gameState);
//
//EntityIndex CreateNewMonster(GameState* gameState);

class GameRuntime
{
public:
	void LoadAllContent();

	void UnloadAllContent();

	void StartGame();

	void Update();

	GameState state;
};
