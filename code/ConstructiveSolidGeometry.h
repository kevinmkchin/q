#pragma once

#include <vector>

#include <gmath.h>

#include "Common.h"

struct CSGPlane
{
    vec3 n; // normal of the plane
    float d; // distance from origin

    // probably need a unique identifier
};

bool IsPointContainedInHalfSpace(vec3 planeNormal, float planeDist, vec3 point);

bool IsPointOnPlane(vec3 planeNormal, float planeDist, vec3 point);

struct CSGBrush
{
    std::vector<CSGPlane> halfspaces; // one face per plane

    // vertices information (point cloud?)
    // edge information (pairs of vertices?)
    // face/side polygon information (also need link to the plane the face lies on)

};

std::vector<vec3> CSGBrushCalculateVerticesFromHalfSpaces(const CSGBrush& brush);
void DoSmth();