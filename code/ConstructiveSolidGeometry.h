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


    // place points and extrude tool: extrude in direction of normal (cross product) thus extrude only available when placed points are 2D. 
    //                                if points are in a 3D cloud, then do a bunch of cross products to get a bunch of planes, clean up unused or duplicate planes,
    //                                then update intersections,faces,vertices,etc.
    // clip tool: add new plane (the slice), update intersection & vertices etc information, then remove all planes that no longer have a face/vertices
    // vertex tool: move vertices and add planes accordingly (adding planes might just be splitting existing planes into two), then update intersection i think
    //              may need to merge vertices or planes together into one if they become same i.e. if two planes become same, prob need to combine into one plane
    // edge tool: vertex tool but with multiple vertices selected
    // face tool: vertex tool but with multiple vertices selected
    // if multiple brushes are selected, and we want to move an vertex/edge, same vertices between brushes should be treated as one

    // vertices information (point cloud?)
    // edge information (pairs of vertices?)
    // face/side polygon information (also need link to the plane the face lies on)

};

std::vector<vec3> CSGBrushCalculateVerticesFromHalfSpaces(const CSGBrush& brush);
std::vector<float> DoSmth();