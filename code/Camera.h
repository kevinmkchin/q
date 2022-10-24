#pragma once

#include <gmath.h>

struct GameCamera
{
    vec3  position;             // camera x y z position in world space
    vec3  rotation;             // roll yaw pitch - in that order
    vec3  calculatedDirection;  // Intuitive direction - direction forward
    vec3  calculatedUp;
    vec3  calculatedRight;
    mat4  perspectiveMatrix;    // Perspective projection matrix
    mat4  viewMatrix;           // Last calculated view matrix
    float nearClip = 0.1f;
    float farClip = 10000.f;

    void CameraUpdate(vec3 newPosition, float yawDelta, float pitchDelta);
    void CameraCalculatePerspectiveMatrix();
    void CameraCalculateViewMatrix();

    // Transforms world space position into screen space position using gameLayer resolution
    // Screenspace is defined in pixels. The top-left of the screen is (0,0); the bottom-right is (pixelWidth,pixelHeight). 
    // The returned z position is in world units from the camera.
    vec3 WorldToScreenPoint(vec3 worldPosition);

    // Get the worldspace point created by converting the screen space point at the provided distance z from the camera plane.
    vec3 ScreenToWorldPoint(vec3 screenPosition);

    float WorldPointToCameraDistance(vec3 worldPosition);
};
