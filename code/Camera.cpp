#include "Camera.h"

#include <gmath.h>

#include "Engine.h"
#include "GameRenderer.h"
#include "Input.h"

void GameCamera::CameraUpdate(vec3 newPosition, float yawDelta, float pitchDelta)
{
    rotation.y -= yawDelta;
    rotation.z -= pitchDelta;

    position = newPosition;

    if (rotation.z > 89.f)
    {
        rotation.z = 89.f;
    }
    if (rotation.z < -89.f)
    {
        rotation.z = -89.f;
    }

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

void GameCamera::CameraCalculateViewMatrix()
{
    viewMatrix = ViewMatrixLookAt(position, position + calculatedDirection, calculatedUp);
}