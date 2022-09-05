#include "GameEntities.h"



Player::Player() 
	: graphics(GameModelComponent(&transform))
	, collider(MeshCollider(self, &transform, std::bind(&Player::OnCollision, this, std::placeholders::_1)))
{
}

void Player::Update()
{
	
}

void Player::OnCollision(CollisionEventArgs event)
{
	PrintLog.Message("player has collided");
}


void Monster::Update()
{

}

RotatingCube::RotatingCube()
	: graphics(GameModelComponent(&transform))
{
}

void RotatingCube::Update()
{
	LOCAL_PERSIST vec3 euler = vec3();
	euler += vec3(0.6f, 0.4f, 0.3f) * Time.deltaTime;
	transform.orientation = EulerToQuat(euler);
}
