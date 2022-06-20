#pragma once

#include "Common.h"
#include "GameComponents.h"

class Entity
{
public:
	virtual void Update() = 0;

public:
	TransformComponent transform;
	EntityIndex self = EntityIndexInvalid;
};

class Player : public Entity
{
public:
	Player();

	virtual void Update() override;

	void OnCollision(CollisionEventArgs event);

public:
	GameModelComponent graphics;

	MeshCollider collider;
};

class Monster : public Entity
{
public:
	virtual void Update() override;
};

class RotatingCube : public Entity
{
public:
	RotatingCube();

	virtual void Update() override;

public:
	GameModelComponent graphics;

};
