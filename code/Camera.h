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
    float farClip = 64000.f;
    float turnSpeed = 0.04f;

    void CameraUpdate(vec3 newPosition, float yawDelta, float pitchDelta);
    void CameraCalculatePerspectiveMatrix();
    void CameraCalculateViewMatrix();
};
