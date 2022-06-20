#include "GameComponents.h"



mat4 TransformComponent::GetModelMatrix()
{
	mat4 modelMatrix = mat4();
	modelMatrix *= TranslationMatrix(position);
	modelMatrix *= RotationMatrix(orientation);
	modelMatrix *= ScaleMatrix(scale);
	return modelMatrix;
}

GameModelComponent::GameModelComponent(TransformComponent* transformArg)
	: transform(transformArg)
	, sharedMesh(nullptr)
{
}
