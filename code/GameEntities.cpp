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
