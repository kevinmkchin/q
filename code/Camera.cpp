#include "Camera.h"

#include <gmath.h>

#include "Engine.h"
#include "GameRenderer.h"
#include "Input.h"

void GameCamera::CameraUpdate()
{
    //if (bMouseControl)
    //{
    //    rotation.y /*yaw*/ -= ((float)gMouseDelta.x) * turnSpeed;
    //    rotation.z /*pitch*/ -= ((float)gMouseDelta.y) * turnSpeed;
    //}

    //if (gGamepadStates[0].isConnected)
    //{
    //    rotation.y -= gGamepadStates[0].rightThumbStickDir.x * turnSpeed * gDeltaTime * 1100.f;
    //    rotation.z += gGamepadStates[0].rightThumbStickDir.y * turnSpeed * gDeltaTime * 1100.f;
    //}

    float moveSpeed = 1.f;

    if (Input::IsKeyPressed(SDL_SCANCODE_W))
    {
        position += calculatedDirection * moveSpeed * Time.deltaTime;
    }
    if (Input::IsKeyPressed(SDL_SCANCODE_A))
    {
        position += -calculatedRight * moveSpeed * Time.deltaTime;
    }
    if (Input::IsKeyPressed(SDL_SCANCODE_S))
    {
        position += -calculatedDirection * moveSpeed * Time.deltaTime;
    }
    if (Input::IsKeyPressed(SDL_SCANCODE_D))
    {
        position += calculatedRight * moveSpeed * Time.deltaTime;
    }
    if (Input::IsKeyPressed(SDL_SCANCODE_Q))
    {
        position.y += -moveSpeed * Time.deltaTime;
    }
    if (Input::IsKeyPressed(SDL_SCANCODE_E))
    {
        position.y += moveSpeed * Time.deltaTime;
    }

    if (rotation.z > 89.f)
    {
        rotation.z = 89.f;
    }
    if (rotation.z < -89.f)
    {
        rotation.z = -89.f;
    }

    // Calcuate direction, right, and up vectors
    calculatedDirection = OrientationToDirection(EulerToQuat(rotation * GM_DEG2RAD));
    calculatedDirection = Normalize(calculatedDirection);
    calculatedRight = Normalize(Cross(calculatedDirection, GM_UP_VECTOR));
    calculatedUp = Normalize(Cross(calculatedRight, calculatedDirection));

    CameraCalculateViewMatrix();
}

void GameCamera::CameraCalculatePerspectiveMatrix()
{
    float internalResolutionWidth = (float) Engine.gfx->gameLayer.width;
    float internalResolutionHeight = (float) Engine.gfx->gameLayer.height;
    float aspect_ratio = internalResolutionWidth / internalResolutionHeight;
    perspectiveMatrix = ProjectionMatrixPerspective((90.f / 2.f) * GM_DEG2RAD, aspect_ratio, nearClip, farClip); // TODO(Kevin): FOV
}

void GameCamera::CameraCalculatePerspectiveMatrix(float aspectRatio)
{
    perspectiveMatrix = ProjectionMatrixPerspective((90.f / 2.f) * GM_DEG2RAD, aspectRatio, nearClip, farClip); // TODO(Kevin): FOV
}

void GameCamera::CameraCalculateViewMatrix()
{
    viewMatrix = ViewMatrixLookAt(position, position + calculatedDirection, calculatedUp);
}