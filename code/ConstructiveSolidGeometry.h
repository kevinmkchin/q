#pragma once

#include <vector>
#include <utility>
#include <unordered_map>

#include <gmath.h>

#include "Common.h"
#include "LevelGeometry.h"

struct CSGPlane
{
    vec3 n; // normal of the plane
    float d; // distance from origin
};

bool IsPointContainedInHalfSpace(vec3 planeNormal, float planeDist, vec3 point);

bool IsPointOnPlane(vec3 planeNormal, float planeDist, vec3 point);

struct CSGBrush
{
    std::vector<CSGPlane> halfspaces; // one face per plane
    std::vector<vec3> pointCloud;
    std::vector<std::vector<CSGPlane>> pointToPlanes; // when we move vertices, this is going to be used to find which planes we need to modify
    std::unordered_map<int, std::vector<vec3>> planeToVertices;
    std::unordered_map<int, std::vector<std::pair<vec3, vec3>>> planeToEdges;
    
    std::vector<LevelGeometry::FaceBatch> facesForDrawing;
    
    // face information

    
    /// TODO(Kevin)
    ///
    /// -selected brush shows up as translucent (only translucent to show its own edges, not brushes behind it)
    /// -brush edges show up in red
    /// -brush surfaces are highlighted in red
    /// -yellow filled circular handles show up; occluded circles are darker
    /// -hover mouse near circular handle to show bigger red unfilled circle around yellow circle; hover one at a time (follow z order)
    /// -yellow circular handle turns red upon click and release
    /// -upon click, user can drag mouse to move circle/vertex
    /// -with vertex tool, clicking and holding down left click on circle shows translucent yellow lines along the axes
    ///
    /// -update intersections function
    /// -face tool and edge tool is just vertex tool w diff num of vertices selected
    ///
    /// -the things that move show up in yellow, so vertices if vertex mode, edges if edge mode, faces if face mode
    
    // place points and extrude tool: extrude in direction of normal (cross product) thus extrude only available when placed points are 2D. 
    //                                if points are in a 3D cloud, then do a bunch of cross products to get a bunch of planes, clean up unused or duplicate planes,
    //                                then update intersections,faces,vertices,etc.
    // clip tool: add new plane (the slice), update intersection & vertices etc information, then remove all planes that no longer have a face/vertices
    // vertex tool: move vertices and add planes accordingly (adding planes might just be splitting existing planes into two), then update intersection i think
    //              may need to merge vertices or planes together into one if they become same i.e. if two planes become same, prob need to combine into one plane
    // edge tool: vertex tool but with multiple vertices selected
    // face tool: vertex tool but with multiple vertices selected
    // if multiple brushes are selected, and we want to move an vertex/edge, same vertices between brushes should be treated as one

};

void UpdateBrushVerticesEdgesDataFromHalfSpaces(CSGBrush* brush);
void UpdateBrushFacesForDrawing(CSGBrush* brush);

CSGBrush* DoSmth();
