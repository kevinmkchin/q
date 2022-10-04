#include "ConstructiveSolidGeometry.h"

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

void DoSmth()//CSGBrush& brush)
{
    CSGPlane p0;
    p0.n = Normalize(vec3(1, 0, 0));
    p0.d = 1.f;
    CSGPlane p1;
    p1.n = Normalize(vec3(0, 1, 0));
    p1.d = 1.f;
    CSGPlane p2;
    p2.n = Normalize(vec3(0, 0, 1));
    p2.d = 1.f;
    CSGPlane p3;
    p3.n = Normalize(vec3(-1, 0, 0));
    p3.d = 1.f;
    CSGPlane p4;
    p4.n = Normalize(vec3(0, -1, 0));
    p4.d = 1.f;
    CSGPlane p5;
    p5.n = Normalize(vec3(0, 0, -1));
    p5.d = 1.f;

    CSGPlane slice;
    slice.n = vec3(0,1,0);
    slice.d = 0.1f;

    CSGBrush brush;
    brush.halfspaces.push_back(p0);
    brush.halfspaces.push_back(p1);
    brush.halfspaces.push_back(p2);
    brush.halfspaces.push_back(p3);
    brush.halfspaces.push_back(p4);
    brush.halfspaces.push_back(p5);
    brush.halfspaces.push_back(slice);

    std::vector<vec3> vertices = CSGBrushCalculateVerticesFromHalfSpaces(brush);

    for (auto v : vertices)
    {
        PrintLog.MessageFmt("x: {}, y: {}, z: {}", v.x, v.y, v.z);
    }

    for (auto p : brush.halfspaces)
    {
        PrintLog.Message("---");
        std::vector<vec3> verticesOnThisPlane;
        for (auto v : vertices)
        {
            if (IsPointOnPlane(p.n, p.d, v))
            {
                verticesOnThisPlane.push_back(v);
            }
        }

        for (auto v : verticesOnThisPlane)
        {
            PrintLog.MessageFmt("x: {}, y: {}, z: {}", v.x, v.y, v.z);
        }
    }

    // we know x and y connect
    // we know z and y connect
    // we know w and z connect
    // we know x and w connect

}