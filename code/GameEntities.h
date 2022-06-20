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

class Player : public virtual Entity
{
public:
	Player();

	virtual void Update() override;

	void OnCollision(CollisionEventArgs event);

public:
	GameModelComponent graphics;

	MeshCollider collider;
};

class Monster : public virtual Entity
{
public:
	virtual void Update() override;
};