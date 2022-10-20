#include "ConstructiveSolidGeometry.h"

#include <unordered_map>
#include <algorithm>

#define EPS_NORMAL 0.001f
#define EPS_WCS 0.1f

bool IsPointContainedInHalfSpace(vec3 planeNormal, float planeDist, vec3 point)
{
	return Dot(point, planeNormal) <= planeDist + EPS_WCS;
}

bool IsPointOnPlane(vec3 planeNormal, float planeDist, vec3 point)
{
    float magnitudeOfPointProjectionToNormal = Dot(point, planeNormal);
    return planeDist - EPS_WCS < magnitudeOfPointProjectionToNormal && magnitudeOfPointProjectionToNormal < planeDist + EPS_WCS;
}

void MovePointOnBrushAndUpdateHalfSpaces(vec3 point, vec3 newPoint, CSGBrush* brush, vec3* ye)
{
    /* For each existing plane, need to if vector from point to newPoint crossed that plane
     * and if so where it intersects -> this intersect point should be newPoint */

    vec3 vecToNewPoint = newPoint - point;
    vec3 vecToNewPointNormalized = Normalize(vecToNewPoint);
    for (auto p : brush->halfspaces)
    {
        vec3 l0 = point;
        vec3 l = vecToNewPointNormalized;
        float ldotn = Dot(l, p.n);
        if (ldotn > EPS_NORMAL) // only if same direction as plane normal? AND not zero
        {
            vec3 p0 = p.n * p.d;
            float dScalar = Dot((p0 - l0), p.n) / ldotn;
            vec3 pointOfIntersection = l0 + l * dScalar;

            if (!pointOfIntersection.Equals(point, EPS_WCS)
                && Magnitude(pointOfIntersection - point) < Magnitude(vecToNewPoint))
            {
                newPoint = pointOfIntersection;
                *ye = pointOfIntersection;
                break; // NOTE(Kevin): there may be multiple points of intersection before original newPoint..
            }
        }
    }

    // don't care about updating anything other than the array of halfspaces
    // unless there are some quick updates we can make that will allow us to optimize moving multiple vertices at once
    int indexOfPoint = -1;
    for (int i = 0; i < brush->pointCloud.size(); ++i)
    {
        if (brush->pointCloud[i].Equals(point, EPS_WCS))
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
            if(iter.Equals(plane, EPS_NORMAL))
            {
                indexOfPlane = i;
                break;
            }
        }

        /* When a vertex belonging to a plane/halfspace with more than 3 vertices is moved, then one
         * of three cases will occur: the vertex moves inside the halfspace, the vertex moves
         * outside the halfspace (no longer contained by the halfspace), or the vertex remains on the
         * plane (the vertex moved perpendicular to the normal of the plane). In the first case, only
         * one new plane is created (the plane formed from the moved vertex and its neighbours). In
         * the second case, a new plane is created for every triangle formed from the moved vertex
         * and the edges of the original plane that aren't edges formed with the old position of the
         * moved vertex. In the third case, no new planes are created. */

        bool planeCreatedForEveryNonNeighbourEdge = true;
        if (brush->planeToVertices[indexOfPlane].size() > 3)
        {
            bool onPlane = IsPointOnPlane(plane.n, plane.d, newPoint);
            bool contained = IsPointContainedInHalfSpace(plane.n, plane.d, newPoint);
            planeCreatedForEveryNonNeighbourEdge = onPlane || !contained;
        }

        if(planeCreatedForEveryNonNeighbourEdge)
        {
            for (int e = 0; e < brush->planeToEdges[indexOfPlane].size(); ++e)
            {
                std::pair<vec3,vec3> edge = brush->planeToEdges[indexOfPlane][e];
                if (!point.Equals(edge.first, EPS_WCS)
                && !point.Equals(edge.second, EPS_WCS)
                && !newPoint.Equals(edge.first, EPS_WCS)
                && !newPoint.Equals(edge.second, EPS_WCS))
                {
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
                        if (iter.Equals(createdPlane, EPS_NORMAL))
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
        else
        {
            /* We know that only one *new* plane is added (in addition to the existing plane
             * defined by the vertices excluding the moved plane. Therefore, we add the old
             * existing plane, then add the newly created plane from the moved vertex and its
             * neighbouring vertices. */

            vec3 a = newPoint;
            vec3 b = vec3();
            vec3 c = vec3();
            bool bHasValue = false;
            for (int e = 0; e < brush->planeToEdges[indexOfPlane].size(); ++e)
            {
                std::pair<vec3,vec3> edge = brush->planeToEdges[indexOfPlane][e];
                vec3 otherOfEdge = vec3();
                bool otherOfEdgeHasValue = false;
                if (point.Equals(edge.first, EPS_WCS))
                {
                    otherOfEdge = edge.second;
                    otherOfEdgeHasValue = true;
                }
                else if (point.Equals(edge.second, EPS_WCS))
                {
                    otherOfEdge = edge.first;
                    otherOfEdgeHasValue = true;
                }
                if (otherOfEdgeHasValue && !bHasValue)
                {
                    b = otherOfEdge;
                    bHasValue = true;
                }
                else if (otherOfEdgeHasValue && bHasValue)
                {
                    c = otherOfEdge;
                }
            }
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

            bool oldPlaneExist = false;
            bool newlyCreatedPlaneExist = false;
            for (auto iter : newlyCreatedPlanes)
            {
                if (iter.Equals(plane, EPS_NORMAL))
                {
                    oldPlaneExist = true;
                }
                if (iter.Equals(createdPlane, EPS_NORMAL))
                {
                    newlyCreatedPlaneExist = true;
                }
            }
            if (!oldPlaneExist) newlyCreatedPlanes.push_back(plane);
            if (!newlyCreatedPlaneExist) newlyCreatedPlanes.push_back(createdPlane);
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
            if (iter.Equals(pl, EPS_NORMAL))
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

    for (auto p : newlyCreatedPlanes)
    {
        if (p.n.Equals(vec3(0.f, 0.f, 0.f), EPS_NORMAL)) continue;

        bool samePlaneExists = false;
        for (auto existingP : brush->halfspaces)
        {
            if (existingP.Equals(p, EPS_NORMAL))
            {
                samePlaneExists = true;
            }
        }
        if (samePlaneExists) continue;

        brush->halfspaces.push_back(p);
    }

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

                if (GM_abs(t) <= EPS_NORMAL) continue; // the planes are parallel

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
                for (int c = 0; c < brush->pointCloud.size(); ++c)
                {
                    if(brush->pointCloud[c].Equals(intersectionPoint, EPS_WCS))
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

            for (auto vertI : verticesOfPlaneI)
            {
                for (auto vertJ : verticesOfPlaneJ)
                {
                    if (vertI.Equals(vertJ, EPS_WCS))
                    {
                        commonVertices.push_back(vertI);
                    }
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
            if (!pickedVert.Equals(edge.first, EPS_WCS) && !pickedVert.Equals(edge.second, EPS_WCS))
            {
                float uvScale = (1.f / 256.f);

                // then there is a tri between pickedVert, edge.first, and edge.second
                vec3 a = pickedVert;
                vec3 b = edge.first;
                vec3 c = edge.second;
                vec3 ba = b - a;
                vec3 ca = c - a;
                vec3 n = Normalize(Cross(ba, ca));

                if (Dot(n, brush->halfspaces[i].n) <= 0.f) // need to ensure winding order of a b c is counter clockwise (opengl default)
                {
                    vec3 t = b;
                    b = c;
                    c = t; 
                    n = -n;
                }

                float xf = GM_abs(Dot(n, vec3(1.f, 0.f, 0.f)));
                float yf = GM_abs(Dot(n, vec3(0.f, 1.f, 0.f)));
                float zf = GM_abs(Dot(n, vec3(0.f, 0.f, 1.f)));

                float au;
                float av;
                float bu;
                float bv;
                float cu;
                float cv;

                if(xf >= yf && xf >= zf)
                {
                    au = a.z * uvScale;
                    av = a.y * uvScale;
                    bu = b.z * uvScale;
                    bv = b.y * uvScale;
                    cu = c.z * uvScale;
                    cv = c.y * uvScale;
                }
                else if(yf >= xf && yf >= zf)
                {
                    au = a.x * uvScale;
                    av = a.z * uvScale;
                    bu = b.x * uvScale;
                    bv = b.z * uvScale;
                    cu = c.x * uvScale;
                    cv = c.z * uvScale;
                }
                else if(zf >= xf && zf >= yf)
                {
                    au = a.x * uvScale;
                    av = a.y * uvScale;
                    bu = b.x * uvScale;
                    bv = b.y * uvScale;
                    cu = c.x * uvScale;
                    cv = c.y * uvScale;
                }

                vertexBuffer.push_back(a.x);
                vertexBuffer.push_back(a.y);
                vertexBuffer.push_back(a.z);
                vertexBuffer.push_back(au);
                vertexBuffer.push_back(av);
                vertexBuffer.push_back(brush->halfspaces[i].n.x);
                vertexBuffer.push_back(brush->halfspaces[i].n.y);
                vertexBuffer.push_back(brush->halfspaces[i].n.z);
                vertexBuffer.push_back(b.x);
                vertexBuffer.push_back(b.y);
                vertexBuffer.push_back(b.z);
                vertexBuffer.push_back(bu);
                vertexBuffer.push_back(bv);
                vertexBuffer.push_back(brush->halfspaces[i].n.x);
                vertexBuffer.push_back(brush->halfspaces[i].n.y);
                vertexBuffer.push_back(brush->halfspaces[i].n.z);
                vertexBuffer.push_back(c.x);
                vertexBuffer.push_back(c.y);
                vertexBuffer.push_back(c.z);
                vertexBuffer.push_back(cu);
                vertexBuffer.push_back(cv);
                vertexBuffer.push_back(brush->halfspaces[i].n.x);
                vertexBuffer.push_back(brush->halfspaces[i].n.y);
                vertexBuffer.push_back(brush->halfspaces[i].n.z);
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
    p0.d = 128.f;
    CSGPlane p1;
    p1.n = Normalize(vec3(0, 1, 0));
    p1.d = 128.f;
    CSGPlane p2;
    p2.n = Normalize(vec3(0, 0, 1));
    p2.d = 128.f;
    CSGPlane p3;
    p3.n = Normalize(vec3(-1, 0, 0));
    p3.d = 128.f;
    CSGPlane p4;
    p4.n = Normalize(vec3(0, -1, 0));
    p4.d = 128.f;
    CSGPlane p5;
    p5.n = Normalize(vec3(0, 0, -1));
    p5.d = 128.f;

    CSGPlane slice;
    slice.n = Normalize(vec3(1,1,0));
    slice.d = 128.f;

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
