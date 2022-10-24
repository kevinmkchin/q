#pragma once

#include <gmath.h>

namespace Handles
{
    void Init();

    void Draw(const mat4* projectionMatrix, const mat4* viewMatrix);

    void DrawSolidDisc(vec3 center, vec3 normal, float radius);

    void DrawLine(vec3 p1, vec3 p2);

    void DrawLine(vec3 p1, vec3 p2, float thickness);

    // // Record a distance measurement from a handle.
    // // All handles call this with their controlID during layout, then use nearestControl to check if they got the mouseDown.
    // void AddControl(int controlId, float distance);

    // Pixel distance from mouse pointer to camera facing circle
    float DistanceToCircle(vec3 center, float radius);

    // // Pixel distance from a point p in 2d to a line defined by two points a and b.
    // void DistanceToLine(vec2 p, vec2 a, vec2 b);

    // // WCS point position to screen position
    // void WorldToScreenCoordPoint(vec3 position);

    // Get a suitable screen size for the handle, based on the distance 
    // from the handle's position to the active game camera.
    float GetHandleSize(vec3 position, float sizeInPixels);
}