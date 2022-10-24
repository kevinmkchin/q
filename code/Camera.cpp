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

vec3 GameCamera::WorldToScreenPoint(vec3 worldPosition)
{
    vec4 clipspaceCoordinates = perspectiveMatrix * viewMatrix * vec4(worldPosition, 1.f);
    float screenspaceRatioX = ((clipspaceCoordinates.x / clipspaceCoordinates.w) + 1.f) / 2.f;
    float screenspaceRatioY = 1.f - (((clipspaceCoordinates.y / clipspaceCoordinates.w) + 1.f) / 2.f);

    float internalResolutionWidth = (float)Engine.gfx->gameLayer.width;
    float internalResolutionHeight = (float)Engine.gfx->gameLayer.height;

    float distanceFromCameraWCS = WorldPointToCameraDistance(worldPosition);

    return vec3(screenspaceRatioX * internalResolutionWidth, 
        screenspaceRatioY * internalResolutionHeight,
        distanceFromCameraWCS);
}

vec3 GameCamera::ScreenToWorldPoint(vec3 screenPosition)
{
    float irw = (float)Engine.gfx->gameLayer.width;
    float irh = (float)Engine.gfx->gameLayer.height;
    mat4 inverseProjViewMat = (perspectiveMatrix * viewMatrix).GetInverse();
    vec4 in = vec4();
    in.x = (screenPosition.x / irw) * 2.f - 1.f;
    in.y = 1.f - ((screenPosition.y / irh) * 2.f);
    in.z = 0.95f;
    in.w = 1.f;
    vec4 pointOnCameraPlane = inverseProjViewMat * in;
    pointOnCameraPlane.xyz *= 1.f / pointOnCameraPlane.w;

    vec3 dir = pointOnCameraPlane.xyz - position;
    float projOfVectorToPointUntoCameraDirection = Dot(dir, calculatedDirection);
    float ratioOfDesiredDistanceToActual = screenPosition.z / projOfVectorToPointUntoCameraDirection;
    return position + dir * ratioOfDesiredDistanceToActual;
}

float GameCamera::WorldPointToCameraDistance(vec3 worldPosition)
{
    return Dot(worldPosition - position, calculatedDirection);
}
