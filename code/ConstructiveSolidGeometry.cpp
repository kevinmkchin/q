#include "ConstructiveSolidGeometry.h"

#include <unordered_map>
#include <algorithm>

#define EPSILON 0.001f // NOTE(Kevin): Not sure if this is large enough...

bool IsPointContainedInHalfSpace(vec3 planeNormal, float planeDist, vec3 point)
{
	return Dot(point, planeNormal) <= planeDist + EPSILON;
}

bool IsPointOnPlane(vec3 planeNormal, float planeDist, vec3 point)
{
    float magnitudeOfPointProjectionToNormal = Dot(point, planeNormal);
    return planeDist - EPSILON < magnitudeOfPointProjectionToNormal && magnitudeOfPointProjectionToNormal < planeDist + EPSILON;
}

void MovePointOnBrushAndUpdateHalfSpaces(vec3 point, vec3 newPoint, CSGBrush* brush)
{
    // don't care about updating anything other than the array of halfspaces
    // unless there are some quick updates we can make that will allow us to optimize moving multiple vertices at once
    int indexOfPoint = -1;
    for (int i = 0; i < brush->pointCloud.size(); ++i)
    {
        if (brush->pointCloud[i].Equals(point, EPSILON))
        {
            indexOfPoint = i;
            break;
        }
    }
    if (indexOfPoint < 0) return;
    
    std::vector<CSGPlane> relevantPlanes = brush->pointToPlanes[indexOfPoint];

    std::vector<CSGPlane> newlyCreatedPlanes;
 
    for (CSGPlane plane : relevantPlanes)
    {
        int indexOfPlane = -1;
        for(int i = 0; i < brush->halfspaces.size(); ++i)
        {
            CSGPlane iter = brush->halfspaces[i];
            if(iter.Equals(plane, EPSILON))
            {
                indexOfPlane = i;
                break;
            }
        }
        // std::vector<vec3> verticesOfPlane = brush->planeToVertices[indexOfPlane];
        // point belongs on this vertices of plane, replace point with new point, then calculate new set of planes
        // NO, its same shit as triangulation -> this old point becomes the picked point, then 
        for (int e = 0; e < brush->planeToEdges[indexOfPlane].size(); ++e)
        {
            std::pair<vec3,vec3> edge = brush->planeToEdges[indexOfPlane][e];
            if (point.Equals(edge.first, EPSILON) == false && point.Equals(edge.second, EPSILON) == false)
            {
                // then there is a plane between pickedVert, edge.first, and edge.second
                vec3 a = newPoint;
                vec3 b = edge.first;
                vec3 c = edge.second;
                vec3 ba = b - a;
                vec3 ca = c - a;
                vec3 crossProduct = Normalize(Cross(ba, ca));

                CSGPlane createdPlane;
                createdPlane.n = crossProduct;
                createdPlane.d = Dot(newPoint, crossProduct);
                if (Dot(crossProduct, plane.n) < 0.f) // wrong direction as old plane
                {
                    createdPlane.n = -createdPlane.n;
                    createdPlane.d = -createdPlane.d;
                }

                // add new plane to list of new planes unless this new plane is already added
                bool sameExist = false;
                for (auto iter : newlyCreatedPlanes)
                {
                    if (iter.Equals(createdPlane, EPSILON))
                    {
                        sameExist = true;
                        break;
                    }
                }
                if(!sameExist)
                {
                    newlyCreatedPlanes.push_back(createdPlane);
                }
            }
        }
    }

    // remove relevant planes from brush->halfspaces
    // add unique set of new planes to brush->halfspaces
    for(auto pl : relevantPlanes)
    {
        bool sameExist = false;
        int indexToRemoveAt = -1;
        for (int i = 0; i < brush->halfspaces.size(); ++i)
        {
            CSGPlane iter = brush->halfspaces[i];
            if (iter.Equals(pl, EPSILON))
            {
                sameExist = true;
                indexToRemoveAt = i;
                break;
            }
        }
        if(sameExist)
        {
            brush->halfspaces[indexToRemoveAt] = brush->halfspaces.back();
            brush->halfspaces.pop_back();
        }
    }
    brush->halfspaces.insert(brush->halfspaces.end(), newlyCreatedPlanes.begin(), newlyCreatedPlanes.end());

    // we know that, as long as brush->halfspaces is correct, all subsequent vectors and calculations will be correct
    brush->pointCloud.clear();
    brush->pointToPlanes.clear();
    brush->planeToVertices.clear();
    brush->planeToEdges.clear();
}

void UpdateBrushVerticesEdgesDataFromHalfSpaces(CSGBrush* brush)
{
    int halfspacesN = (int)brush->halfspaces.size();
    for (int i = 0; i < halfspacesN - 2; ++i)
    {
        for (int j = i + 1; j < halfspacesN - 1; ++j)
        {
            for (int k = j + 1; k < halfspacesN; ++k)
            {
                CSGPlane pi = brush->halfspaces[i];
                CSGPlane pj = brush->halfspaces[j];
                CSGPlane pk = brush->halfspaces[k];

                float t = pi.n.x * (pj.n.y * pk.n.z - pj.n.z * pk.n.y)
                    + pi.n.y * (pj.n.z * pk.n.x - pj.n.x * pk.n.z)
                    + pi.n.z * (pj.n.x * pk.n.y - pj.n.y * pk.n.x);

                if (GM_abs(t) <= EPSILON) continue; // the planes are parallel

                float vx = (pi.d * (pj.n.z * pk.n.y - pj.n.y * pk.n.z) + pj.d * (pi.n.y * pk.n.z - pi.n.z * pk.n.y) + pk.d * (pi.n.z * pj.n.y - pi.n.y * pj.n.z)) / -t;
                float vy = (pi.d * (pj.n.x * pk.n.z - pj.n.z * pk.n.x) + pj.d * (pi.n.z * pk.n.x - pi.n.x * pk.n.z) + pk.d * (pi.n.x * pj.n.z - pi.n.z * pj.n.x)) / -t;
                float vz = (pi.d * (pj.n.y * pk.n.x - pj.n.x * pk.n.y) + pj.d * (pi.n.x * pk.n.y - pi.n.y * pk.n.x) + pk.d * (pi.n.y * pj.n.x - pi.n.x * pj.n.y)) / -t;

                // 8 points, and all should be within every halfspace

                vec3 intersectionPoint = vec3(vx, vy, vz);
                bool containedWithinEveryHalfspace = true;
                for (int m = 0; m < halfspacesN; ++m)
                {
                    CSGPlane pm = brush->halfspaces[m];
                    if (m != i && m != j && m != k && !IsPointContainedInHalfSpace(pm.n, pm.d, intersectionPoint))
                    {
                        containedWithinEveryHalfspace = false;
                        break;
                    }
                }
                
                /* Because we are checking intersection between 3 planes, if a point is formed from
                   intersection of 4+ planes, then the point will be calculated multiple times. Thus,
                   don't add an intersection point if it was already calculated before and added. */
                bool flag_pointAlreadyCalculated = false;
                for(int c = 0; c < brush->pointCloud.size(); ++c)
                {
                    if(brush->pointCloud[c].Equals(intersectionPoint, EPSILON))
                    {
                        flag_pointAlreadyCalculated = true;
                        break;
                    }
                }
                if (flag_pointAlreadyCalculated) continue;

                if (containedWithinEveryHalfspace)
                {
                    brush->pointCloud.push_back(intersectionPoint); // intersectionPoint is a vertex
                    brush->pointToPlanes.push_back(std::vector<CSGPlane>());

                    for(int p = 0; p < halfspacesN; ++p)
                    {
                        CSGPlane pl = brush->halfspaces[p];
                        if(IsPointOnPlane(pl.n, pl.d, intersectionPoint))
                        {
                            brush->pointToPlanes[brush->pointCloud.size() - 1].push_back(pl);
                            brush->planeToVertices[p].push_back(intersectionPoint);
                        }
                    }
                }
            }
        }
    }

    // at this point, 
    //      brush->pointCloud
    //      brush->pointToPlanes 
    //      brush->planeToVertices
    //          are populated
    //

    for (int i = 0; i < halfspacesN - 1; ++i)
    {
        for (int j = i + 1; j < halfspacesN; ++j)
        {
            std::vector<vec3> verticesOfPlaneI = brush->planeToVertices[i];
            std::vector<vec3> verticesOfPlaneJ = brush->planeToVertices[j];

            std::vector<vec3> commonVertices;

            for (auto vert : verticesOfPlaneI)
            {
                if (std::find(verticesOfPlaneJ.begin(), verticesOfPlaneJ.end(), vert) != verticesOfPlaneJ.end())
                {
                    commonVertices.push_back(vert);
                }
            }

            if (commonVertices.size() > 1)
            {
                for (int commonI = 0; commonI < commonVertices.size() - 1; ++commonI)
                {
                    for (int commonJ = commonI + 1; commonJ < commonVertices.size(); ++commonJ)
                    {
                        std::pair<vec3, vec3> commonEdge = std::make_pair(commonVertices[commonI], commonVertices[commonJ]);
                        brush->planeToEdges[i].push_back(commonEdge);
                        brush->planeToEdges[j].push_back(commonEdge);
                    }
                }
            }
        }
    }

    // at this point,
    //      brush->planeToEdges
    //          is also populated
}

void UpdateBrushFacesForDrawing(CSGBrush* brush)
{
    for (LevelGeometry::FaceBatch fb : brush->facesForDrawing)
    {
        LevelGeometry::DeleteFaceBatchMeshOnGPU(fb.idVAO, fb.idVBO);
    }
    brush->facesForDrawing.clear();

    for (int i = 0; i < brush->halfspaces.size(); ++i)
    {
        if (brush->planeToEdges[i].size() < 3) continue;
        std::vector<vec3> verts = brush->planeToVertices[i];
        vec3 pickedVert = verts[0];

        LevelGeometry::FaceBatch face;
        LevelGeometry::CreateFaceBatchMeshOnGPU(&face.idVAO, &face.idVBO);

        std::vector<float> vertexBuffer;

        for (int e = 0; e < brush->planeToEdges[i].size(); ++e)
        {
            std::pair<vec3,vec3> edge = brush->planeToEdges[i][e];
            if (edge.first != pickedVert && edge.second != pickedVert)
            {
                // to get the normal of this vertex, we need to transform from a vec3 WCS point
                // to a vec2 UV coordinate

                // then there is a tri between pickedVert, edge.first, and edge.second
                vec3 a = pickedVert;
                vec3 b = edge.first;
                vec3 c = edge.second;
                vec3 ba = b - a;
                vec3 ca = c - a;
                if (Dot(Normalize(Cross(ba, ca)), brush->halfspaces[i].n) > 0.f)
                {
                    // then add in order a b c
                    vertexBuffer.push_back(a.x);
                    vertexBuffer.push_back(a.y);
                    vertexBuffer.push_back(a.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush->halfspaces[i].n.x);
                    vertexBuffer.push_back(brush->halfspaces[i].n.y);
                    vertexBuffer.push_back(brush->halfspaces[i].n.z);
                    vertexBuffer.push_back(b.x);
                    vertexBuffer.push_back(b.y);
                    vertexBuffer.push_back(b.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush->halfspaces[i].n.x);
                    vertexBuffer.push_back(brush->halfspaces[i].n.y);
                    vertexBuffer.push_back(brush->halfspaces[i].n.z);
                    vertexBuffer.push_back(c.x);
                    vertexBuffer.push_back(c.y);
                    vertexBuffer.push_back(c.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush->halfspaces[i].n.x);
                    vertexBuffer.push_back(brush->halfspaces[i].n.y);
                    vertexBuffer.push_back(brush->halfspaces[i].n.z);
                }
                else
                {
                    // then add in order a c b
                    vertexBuffer.push_back(a.x);
                    vertexBuffer.push_back(a.y);
                    vertexBuffer.push_back(a.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush->halfspaces[i].n.x);
                    vertexBuffer.push_back(brush->halfspaces[i].n.y);
                    vertexBuffer.push_back(brush->halfspaces[i].n.z);
                    vertexBuffer.push_back(c.x);
                    vertexBuffer.push_back(c.y);
                    vertexBuffer.push_back(c.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush->halfspaces[i].n.x);
                    vertexBuffer.push_back(brush->halfspaces[i].n.y);
                    vertexBuffer.push_back(brush->halfspaces[i].n.z);
                    vertexBuffer.push_back(b.x);
                    vertexBuffer.push_back(b.y);
                    vertexBuffer.push_back(b.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush->halfspaces[i].n.x);
                    vertexBuffer.push_back(brush->halfspaces[i].n.y);
                    vertexBuffer.push_back(brush->halfspaces[i].n.z);
                }
            }
        }

        LevelGeometry::RebindFaceBatchBufferObject(&face, (u32) vertexBuffer.size() * sizeof(float), vertexBuffer.data());
        brush->facesForDrawing.push_back(face);
    }
}

CSGBrush* DoSmth()
{
// planes data
    CSGPlane p0;
    p0.n = Normalize(vec3(1, 0, 0));
    p0.d = 100.f;
    CSGPlane p1;
    p1.n = Normalize(vec3(0, 1, 0));
    p1.d = 100.f;
    CSGPlane p2;
    p2.n = Normalize(vec3(0, 0, 1));
    p2.d = 100.f;
    CSGPlane p3;
    p3.n = Normalize(vec3(-1, 0, 0));
    p3.d = 100.f;
    CSGPlane p4;
    p4.n = Normalize(vec3(0, -1, 0));
    p4.d = 100.f;
    CSGPlane p5;
    p5.n = Normalize(vec3(0, 0, -1));
    p5.d = 100.f;

    CSGPlane slice;
    slice.n = Normalize(vec3(1,1,0));
    slice.d = 100.f;

    CSGBrush* brush = new CSGBrush();
    brush->halfspaces.push_back(p0);
    brush->halfspaces.push_back(p1);
    brush->halfspaces.push_back(p2);
    brush->halfspaces.push_back(p3);
    brush->halfspaces.push_back(p4);
    brush->halfspaces.push_back(p5);
    brush->halfspaces.push_back(slice);

    UpdateBrushVerticesEdgesDataFromHalfSpaces(brush);
    UpdateBrushFacesForDrawing(brush);

    return brush;
}
