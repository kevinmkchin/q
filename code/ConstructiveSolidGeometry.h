#pragma once

#include <vector>
#include <utility>
#include <unordered_map>

#include <gmath.h>

#include "Common.h"
#include "LevelGeometry.h"

#define EPS_NORMAL 0.0001f
#define EPS_WCS 0.01f
#define EPS_POINTINPOINTCLOUD 0.1f

struct CSGPlane
{
    vec3 n = vec3(); // normal of the plane
    float d = 0.f; // distance from origin
    
    inline bool Equals(const CSGPlane& other, float epsilon)
    {
        if (GM_abs(d) < 0.01f && GM_abs(other.d) < 0.01f)
        {
            return n.Equals(other.n, epsilon);
        }
        else
        {
            return n.Equals(other.n, epsilon) && d - 0.01f <= other.d && other.d <= d + 0.01f;
        }
    }
};

bool IsPointContainedInHalfSpace(vec3 planeNormal, float planeDist, vec3 point, float eps);

bool IsPointOnPlane(vec3 planeNormal, float planeDist, vec3 point, float eps);

bool FindIntersectionBetweenPlaneAndVector(vec3 planeNormal, float planeDist, vec3 directionVector, vec3 pointOnVector, vec3* out_intersectionPoint);

struct CSGBrush
{
    std::vector<CSGPlane> halfspaces; // one face per plane
    std::vector<vec3> pointCloud;
    std::vector<std::vector<int>> pointToPlanes; // when we move vertices, this is going to be used to find which planes we need to modify
    std::unordered_map<int, std::vector<vec3>> planeToVertices;
    std::unordered_map<int, std::vector<std::pair<vec3, vec3>>> planeToEdges;

    std::vector<LevelGeometry::FaceBatch> facesForDrawing;
};

void MovePointOnBrushAndUpdateHalfSpaces(vec3 point, vec3 newPoint, CSGBrush* brush, vec3* ye);
void UpdateBrushVerticesEdgesDataFromHalfSpaces(CSGBrush* brush);
void UpdateBrushFacesForDrawing(CSGBrush* brush);

bool BrushContainsPoint(CSGBrush* brush, vec3 point, float eps);

CSGBrush* DoSmth();
