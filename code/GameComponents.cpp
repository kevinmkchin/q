#include "GameComponents.h"

#include "Resources.h"


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

void GameModelComponent::Render(const Shader& shader)
{
	if (sharedMesh)
	{
		shader.GLBindMatrix4fv("modelMatrix", 1, transform->GetModelMatrix().ptr());
		sharedMesh->Render();
	}
}
