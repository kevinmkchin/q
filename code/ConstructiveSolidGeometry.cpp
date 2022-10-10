#include "ConstructiveSolidGeometry.h"

#include <unordered_map>

#define EPSILON 0.00001f

bool IsPointContainedInHalfSpace(vec3 planeNormal, float planeDist, vec3 point)
{
	return Dot(point, planeNormal) <= planeDist + EPSILON;
}

bool IsPointOnPlane(vec3 planeNormal, float planeDist, vec3 point)
{
    float magnitudeOfPointProjectionToNormal = Dot(point, planeNormal);
    return planeDist - EPSILON < magnitudeOfPointProjectionToNormal && magnitudeOfPointProjectionToNormal < planeDist + EPSILON;
}

std::vector<vec3> CSGBrushCalculateVerticesFromHalfSpaces(const CSGBrush& brush)
{
    std::vector<vec3> vertices;

    int halfspacesN = (int)brush.halfspaces.size();
    for (int i = 0; i < halfspacesN - 2; ++i)
    {
        for (int j = i + 1; j < halfspacesN - 1; ++j)
        {
            for (int k = j + 1; k < halfspacesN; ++k)
            {
                CSGPlane pi = brush.halfspaces[i];
                CSGPlane pj = brush.halfspaces[j];
                CSGPlane pk = brush.halfspaces[k];

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
                    CSGPlane pm = brush.halfspaces[m];
                    if (m != i && m != j && m != k && !IsPointContainedInHalfSpace(pm.n, pm.d, intersectionPoint))
                    {
                        containedWithinEveryHalfspace = false;
                        break;
                    }
                }
                if (containedWithinEveryHalfspace)
                {
                    vertices.push_back(intersectionPoint); // intersectionPoint is a vertex
                }
            }
        }
    }

    return vertices;
}

std::vector<float> DoSmth()//CSGBrush& brush)
{
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

    CSGBrush brush;
    brush.halfspaces.push_back(p0);
    brush.halfspaces.push_back(p1);
    brush.halfspaces.push_back(p2);
    brush.halfspaces.push_back(p3);
    brush.halfspaces.push_back(p4);
    brush.halfspaces.push_back(p5);
    brush.halfspaces.push_back(slice);

    std::vector<vec3> vertices = CSGBrushCalculateVerticesFromHalfSpaces(brush);
    
    std::unordered_map<int, std::vector<vec3>> planeVertices;
    std::unordered_map<int, std::vector<std::pair<vec3, vec3>>> planeEdges;

    for (int i = 0; i < brush.halfspaces.size(); ++i)
    {
        CSGPlane p = brush.halfspaces[i];
        for (auto v : vertices)
        {
            if (IsPointOnPlane(p.n, p.d, v))
            {
                planeVertices[i].push_back(v);
            }
        }

        // if (verticesOnThisPlane.size() < 3) continue;

        // we know which vertices are on each plane
        // this is enough information to form the face polygon
        // this is enough information to form a collider

        // we only care about which edges/vertices/faces are relevant to which planes ONLY FOR editing purposes
        // bcs we need to know which planes get modified when we modify an edge/face/vertex
    }

    for (int i = 0; i < brush.halfspaces.size() - 1; ++i)
    {
        for (int j = i + 1; j < brush.halfspaces.size(); ++j)
        {
            std::vector<vec3> verticesOfPlaneI = planeVertices[i];
            std::vector<vec3> verticesOfPlaneJ = planeVertices[j];

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
                        planeEdges[i].push_back(commonEdge);
                        planeEdges[j].push_back(commonEdge);
                    }
                }
            }
        }
    }

    // planeEdges and planeVertices are populated
    // we can triangulate now

    std::vector<float> vertexBuffer;

    for (int i = 0; i < brush.halfspaces.size(); ++i)
    {
        if (planeEdges[i].size() < 3) continue;
        std::vector<vec3> verts = planeVertices[i];
        vec3 pickedVert = verts[0];

        for (int e = 0; e < planeEdges[i].size(); ++e)
        {
            std::pair<vec3,vec3> edge = planeEdges[i][e];
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
                if (Dot(Normalize(Cross(ba, ca)), brush.halfspaces[i].n) > 0.f)
                {
                    // then add in order a b c
                    vertexBuffer.push_back(a.x);
                    vertexBuffer.push_back(a.y);
                    vertexBuffer.push_back(a.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush.halfspaces[i].n.x);
                    vertexBuffer.push_back(brush.halfspaces[i].n.y);
                    vertexBuffer.push_back(brush.halfspaces[i].n.z);
                    vertexBuffer.push_back(b.x);
                    vertexBuffer.push_back(b.y);
                    vertexBuffer.push_back(b.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush.halfspaces[i].n.x);
                    vertexBuffer.push_back(brush.halfspaces[i].n.y);
                    vertexBuffer.push_back(brush.halfspaces[i].n.z);
                    vertexBuffer.push_back(c.x);
                    vertexBuffer.push_back(c.y);
                    vertexBuffer.push_back(c.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush.halfspaces[i].n.x);
                    vertexBuffer.push_back(brush.halfspaces[i].n.y);
                    vertexBuffer.push_back(brush.halfspaces[i].n.z);
                }
                else
                {
                    // then add in order a c b
                    vertexBuffer.push_back(a.x);
                    vertexBuffer.push_back(a.y);
                    vertexBuffer.push_back(a.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush.halfspaces[i].n.x);
                    vertexBuffer.push_back(brush.halfspaces[i].n.y);
                    vertexBuffer.push_back(brush.halfspaces[i].n.z);
                    vertexBuffer.push_back(c.x);
                    vertexBuffer.push_back(c.y);
                    vertexBuffer.push_back(c.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush.halfspaces[i].n.x);
                    vertexBuffer.push_back(brush.halfspaces[i].n.y);
                    vertexBuffer.push_back(brush.halfspaces[i].n.z);
                    vertexBuffer.push_back(b.x);
                    vertexBuffer.push_back(b.y);
                    vertexBuffer.push_back(b.z);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(0);
                    vertexBuffer.push_back(brush.halfspaces[i].n.x);
                    vertexBuffer.push_back(brush.halfspaces[i].n.y);
                    vertexBuffer.push_back(brush.halfspaces[i].n.z);
                }
            }
        }
    }

    return vertexBuffer;
}