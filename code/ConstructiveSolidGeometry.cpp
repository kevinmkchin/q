#include "ConstructiveSolidGeometry.h"

#include <unordered_map>
#include <algorithm>

#include "Utils.h"

static bool AddPlaneToUniqueSetOfPlanes(std::vector<CSGPlane>* hsptr, CSGPlane p, float eps)
{
    if (p.n.Equals(vec3(0.f, 0.f, 0.f), 0.f)) return false;
    bool alreadyExist = false;
    for (auto iter : *hsptr)
        if (iter.Equals(p, eps))
            alreadyExist = true;
    if (!alreadyExist) hsptr->push_back(p);
    return !alreadyExist;
}

static bool RemovePlaneFromUniqueSetOfPlanes(std::vector<CSGPlane>* hsptr, CSGPlane p, float eps)
{
    for (int i = 0; i < hsptr->size(); ++i)
    {
        if (hsptr->at(i).Equals(p, eps))
        {
            hsptr->at(i) = hsptr->back();
            hsptr->pop_back();
            return true;
        }
    }
    return false;
}

static void BrushPoint_FindIndexInBrush(std::vector<vec3>* pointCloud, vec3 vToFind, int* out_index, float eps = EPS_POINTINPOINTCLOUD)
{
    int indexOfPoint = -1;
    for (int i = 0; i < pointCloud->size(); ++i)
    {
        if (pointCloud->at(i).Equals(vToFind, eps))
        {
            *out_index = i;
            return;
        }
    }
}

static void BrushPlane_FindIndexInBrush(const std::vector<CSGPlane>* brushPlanes,
                                        CSGPlane vToFind,
                                        int* out_index,
                                        float eps = 0.001f)
{
    for (int i = 0; i < brushPlanes->size(); ++i)
    {
        CSGPlane iter = brushPlanes->at(i);
        if (iter.Equals(vToFind, eps))
        {
            *out_index = i;
            return;
        }
    }
}

static void BrushPlane_FindCommonVertices(CSGBrush* brush, 
                                          int firstPlaneIndex, 
                                          int secondPlaneIndex, 
                                          std::vector<vec3>* out_commonVertices)
{
    std::vector<int> verticesOfPlaneI = brush->planeToVertices[firstPlaneIndex];
    std::vector<int> verticesOfPlaneJ = brush->planeToVertices[secondPlaneIndex];

    for (auto vertI : verticesOfPlaneI)
    {
        for (auto vertJ : verticesOfPlaneJ)
        {
            vec3 vi = brush->pointCloud[vertI];
            vec3 vj = brush->pointCloud[vertJ];
            if (vi.Equals(vj, EPS_WCS))
            {
                out_commonVertices->push_back(vi);
            }
        }
    }
}

bool IsPointContainedInHalfSpace(vec3 planeNormal, float planeDist, vec3 point, float eps)
{
    return Dot(point, planeNormal) <= planeDist + eps;
}

bool IsPointOnPlane(vec3 planeNormal, float planeDist, vec3 point, float eps)
{
    float pointProjectionToNormal = Dot(point, planeNormal);
    return planeDist - eps <= pointProjectionToNormal && pointProjectionToNormal <= planeDist + eps;
}

bool FindIntersectionBetweenPlaneAndVector(vec3 planeNormal, float planeDist, vec3 directionVector, vec3 pointOnVector, vec3* out_intersectionPoint)
{
    vec3 l0 = pointOnVector;
    vec3 l = directionVector;
    float ldotn = Dot(l, planeNormal);
    if (ldotn != 0.f) 
    {
        vec3 p0 = planeNormal * planeDist;
        float dScalar = Dot((p0 - l0), planeNormal) / ldotn;
        vec3 pointOfIntersection = l0 + l * dScalar;
        *out_intersectionPoint = pointOfIntersection;
        return true;
    }
    else // parallel
    {
        return false;
    }
}

void MovePointOnBrushAndUpdateHalfSpaces(vec3 point, vec3 newPoint, CSGBrush* brush, vec3* ye)
{
    // don't care about updating anything other than the array of halfspaces
    // unless there are some quick updates we can make that will allow us to optimize moving multiple vertices at once
    int indexOfPoint = -1;
    BrushPoint_FindIndexInBrush(&brush->pointCloud, point, &indexOfPoint, EPS_POINTINPOINTCLOUD);
    if (indexOfPoint < 0) return;

    point = brush->pointCloud[indexOfPoint];

    if (Magnitude(newPoint - point) < 0.98f)
    {
        *ye = point;
        return;
    }

    std::vector<CSGPlane> affectedFacePlanes;
    for (int index : brush->pointToPlanes[indexOfPoint])
    {
        affectedFacePlanes.push_back(brush->halfspaces[index]);
    }

    /* For each existing plane or potential plane, need to if vector from point to newPoint crossed
     * that plane and if so where it intersects -> this intersect point should be newPoint */

    std::vector<vec3> neighbouringPoints; 

    for (auto p : affectedFacePlanes)
    {
        int indexOfPlane = -1;
        BrushPlane_FindIndexInBrush(&brush->halfspaces, p, &indexOfPlane, EPS_NORMAL);
        for (int e = 0; e < brush->planeToEdges[indexOfPlane].size(); ++e)
        {
            std::pair<int, int> edgeVertexIndices = brush->planeToEdges[indexOfPlane][e];
            std::pair<vec3, vec3> edge = std::make_pair(brush->pointCloud[edgeVertexIndices.first], brush->pointCloud[edgeVertexIndices.second]);
            if (point.Equals(edge.first, EPS_WCS) || point.Equals(edge.second, EPS_WCS))
            {
                vec3 neighbourPoint = point.Equals(edge.first, EPS_WCS) ? edge.second : edge.first;
                bool sameExist = false;
                for (auto iter : neighbouringPoints)
                {
                    if (iter.Equals(neighbourPoint, EPS_NORMAL))
                    {
                        sameExist = true;
                        break;
                    }
                }
                if (!sameExist)
                {
                    neighbouringPoints.push_back(neighbourPoint);
                }
            }
        }
    }

    const std::vector<CSGPlane> existingPlanes = brush->halfspaces;
    std::vector<CSGPlane> potentialIntersectingPlanes;
    for(const CSGPlane p : existingPlanes)
    {
        int check = -1;
        BrushPlane_FindIndexInBrush(&affectedFacePlanes, p, &check);
        if(check < 0)
        {
            potentialIntersectingPlanes.push_back(p);
        }
    }

    vec3 vecToNewPoint = newPoint - point;
    vec3 vecToNewPointNormalized = Normalize(vecToNewPoint);

    // i think main problem of similar planes duplicating happens when i calculate the similar plane
    // so i should try to minimize the number of new plane calculations -> just use existing ones if i know they mergede

    /* For each neighbouring point tuple, form a plane because moving to the new point
     * may intersect with this plane. */
    if (neighbouringPoints.size() >= 3)
    {
        for (int i = 0; i < neighbouringPoints.size() - 2; ++i)
        {
            for (int j = i + 1; j < neighbouringPoints.size() - 1; ++j)
            {
                for (int k = j + 1; k < neighbouringPoints.size(); ++k)
                {
                    vec3 crossProduct = Normalize(Cross(neighbouringPoints[j] - neighbouringPoints[i], neighbouringPoints[k] - neighbouringPoints[i]));
                    CSGPlane intersectionCheckingPlane;
                    intersectionCheckingPlane.n = crossProduct;
                    intersectionCheckingPlane.d = Dot(neighbouringPoints[i], crossProduct);
                    if (Dot(vecToNewPointNormalized, intersectionCheckingPlane.n) < 0.f)
                    {
                        intersectionCheckingPlane.n = -intersectionCheckingPlane.n;
                        intersectionCheckingPlane.d = -intersectionCheckingPlane.d;
                    }
                    AddPlaneToUniqueSetOfPlanes(&potentialIntersectingPlanes, intersectionCheckingPlane, 0.001f);// shouldn't add two same ones?
                }
            }
        }
    }

    float _shortestDistanceAwaySoFar = FLT_MAX;
    for (auto p : potentialIntersectingPlanes)
    {
        vec3 l0 = point;
        vec3 l = vecToNewPointNormalized;
        float ldotn = Dot(l, p.n);
        vec3 pointOfIntersection;
        if (ldotn > EPS_NORMAL // only if same direction as plane normal? AND not zero
            && FindIntersectionBetweenPlaneAndVector(p.n, p.d, l, l0, &pointOfIntersection))
        {
            float iterDist = Magnitude(pointOfIntersection - point);
            if (!pointOfIntersection.Equals(point, EPS_WCS)
                && iterDist < Magnitude(vecToNewPoint))
            {
                if (iterDist - 0.01f <= _shortestDistanceAwaySoFar)
                {
                    if (iterDist + 0.01f < _shortestDistanceAwaySoFar)
                    {
                        newPoint = pointOfIntersection;
                        *ye = pointOfIntersection;
                    }
                }
            }
        }
    }

    std::vector<CSGPlane> newlyCreatedPlanes;

    for (CSGPlane plane : affectedFacePlanes)
    {
        int indexOfPlane = -1;
        BrushPlane_FindIndexInBrush(&existingPlanes, plane, &indexOfPlane, 0.001f);

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
            bool onPlane = IsPointOnPlane(plane.n, plane.d, newPoint, 0.000001f);
            bool contained = IsPointContainedInHalfSpace(plane.n, plane.d, newPoint, 0.000001f);
            planeCreatedForEveryNonNeighbourEdge = onPlane || !contained;
        }

        if(planeCreatedForEveryNonNeighbourEdge)
        {
            for (int e = 0; e < brush->planeToEdges[indexOfPlane].size(); ++e)
            {
                std::pair<int, int> edgeVertexIndices = brush->planeToEdges[indexOfPlane][e];
                std::pair<vec3, vec3> edge = std::make_pair(brush->pointCloud[edgeVertexIndices.first], brush->pointCloud[edgeVertexIndices.second]);
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
                    AddPlaneToUniqueSetOfPlanes(&newlyCreatedPlanes, createdPlane, 0.00005f);
                }
            }
        }
        else
        {
            vec3 a = newPoint;
            vec3 b = vec3();
            vec3 c = vec3();
            bool bHasValue = false;
            bool cHasValue = false;
            for (int e = 0; e < brush->planeToEdges[indexOfPlane].size(); ++e)
            {
                std::pair<int, int> edgeVertexIndices = brush->planeToEdges[indexOfPlane][e];
                std::pair<vec3, vec3> edge = std::make_pair(brush->pointCloud[edgeVertexIndices.first], brush->pointCloud[edgeVertexIndices.second]);
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
                    cHasValue = true;
                }
            }
            if (!bHasValue || !cHasValue) continue;
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
            AddPlaneToUniqueSetOfPlanes(&newlyCreatedPlanes, plane, 0.00005f);
            AddPlaneToUniqueSetOfPlanes(&newlyCreatedPlanes, createdPlane, 0.00005f);
        }
    }

    // remove relevant planes from brush->halfspaces
    // add unique set of new planes to brush->halfspaces
    for(auto pl : affectedFacePlanes)
    {
        RemovePlaneFromUniqueSetOfPlanes(&brush->halfspaces, pl, 0.f);
    }

    for (auto p : newlyCreatedPlanes)
    {
        if (p.n.Equals(vec3(0.f, 0.f, 0.f), EPS_NORMAL))
        {
            printf("breh 000\n");
            continue;
        }
        AddPlaneToUniqueSetOfPlanes(&brush->halfspaces, p, 0.00005f);
    }

    // we know that, as long as brush->halfspaces is correct, all subsequent vectors and calculations will be correct
}

void UpdateBrushVerticesEdgesDataFromHalfSpaces(CSGBrush* brush, vec3* ye)
{
    brush->pointCloud.clear();
    brush->pointToPlanes.clear();
    brush->planeToVertices.clear();
    brush->planeToEdges.clear();
    brush->edgesSet.clear();

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

                if (GM_abs(t) <= 0.000001f) continue; // the planes are parallel

                float vx = (pi.d * (pj.n.z * pk.n.y - pj.n.y * pk.n.z) + pj.d * (pi.n.y * pk.n.z - pi.n.z * pk.n.y) + pk.d * (pi.n.z * pj.n.y - pi.n.y * pj.n.z)) / -t;
                float vy = (pi.d * (pj.n.x * pk.n.z - pj.n.z * pk.n.x) + pj.d * (pi.n.z * pk.n.x - pi.n.x * pk.n.z) + pk.d * (pi.n.x * pj.n.z - pi.n.z * pj.n.x)) / -t;
                float vz = (pi.d * (pj.n.y * pk.n.x - pj.n.x * pk.n.y) + pj.d * (pi.n.x * pk.n.y - pi.n.y * pk.n.x) + pk.d * (pi.n.y * pj.n.x - pi.n.x * pj.n.y)) / -t;

                vec3 intersectionPoint = vec3(vx, vy, vz);
                bool containedWithinEveryHalfspace = true;
                for (int m = 0; m < halfspacesN; ++m)
                {
                    CSGPlane pm = brush->halfspaces[m];
                    if (m != i && m != j && m != k && !IsPointContainedInHalfSpace(pm.n, pm.d, intersectionPoint, 0.01f))
                    {
                        containedWithinEveryHalfspace = false; // intersectionPoint.x * pm.n.x + intersectionPoint.y * pm.n.y + intersectionPoint.z * pm.n.z
                        break;
                    }
                }
                
                /* Because we are checking intersection between 3 planes, if a point is formed from
                   intersection of 4+ planes, then the point will be calculated multiple times. Thus,
                   don't add an intersection point if it was already calculated before and added. */
                if (BrushContainsPoint(brush, intersectionPoint, 0.1f)) continue;

                if (containedWithinEveryHalfspace)
                {
                    brush->pointCloud.push_back(intersectionPoint); // intersectionPoint is a vertex
                    brush->pointToPlanes.push_back(std::vector<int>());

                    for(int p = 0; p < halfspacesN; ++p)
                    {
                        CSGPlane pl = brush->halfspaces[p];
                        if(IsPointOnPlane(pl.n, pl.d, intersectionPoint, 0.01f))
                        {
                            // NOTE(Kevin): When faces disappear, it's likely because either there is a 
                            // incorrect halfspace that makes containedWithinEveryHalfspace = false for most
                            // should-be-valid points, or the number of points in pointCloud is correct but 
                            // there are two similar planes or smth and so planeToVertices gets fucked up
                            int indexOfPointInCloud = int(brush->pointCloud.size() - 1);
                            brush->pointToPlanes[indexOfPointInCloud].push_back(p);
                            brush->planeToVertices[p].push_back(indexOfPointInCloud);
                        }
                    }
                }
            }
        }
    }


    /* For all points, there is a set of immediate neighbouring points. For the set of planes formable by each unique
     * tuple of these neighbouring points, that original point must be outside or on the surface of each of these planes.
     * Otherwise, there is concavity. For those planes for which this criteria is not met are valid to be added as new
     * correct face planes, and the existing planes that contain this bad point must be removed (might need to check
     * if plane only has 3 points). */

//    for (int pointIndex = 0; pointIndex < brush->pointCloud.size(); ++pointIndex)
//    {
//        auto point = brush->pointCloud[pointIndex];
//        std::vector<CSGPlane> relevantPlanes;
//        for (int planeIndex : brush->pointToPlanes[pointIndex])
//        {
//            relevantPlanes.push_back(brush->halfspaces[planeIndex]);
//        }
//        std::vector<vec3> neighbouringPoints;
//        for (auto p : relevantPlanes)
//        {
//            int indexOfPlane = -1;
//            BrushPlane_FindIndexInBrush(&brush->halfspaces, p, &indexOfPlane, EPS_NORMAL);
//            for (int e = 0; e < brush->planeToEdges[indexOfPlane].size(); ++e)
//            {
//                std::pair<int, int> edgeVertexIndices = brush->planeToEdges[indexOfPlane][e];
//                std::pair<vec3, vec3> edge = std::make_pair(brush->pointCloud[edgeVertexIndices.first], brush->pointCloud[edgeVertexIndices.second]);
//                if (point.Equals(edge.first, EPS_WCS) || point.Equals(edge.second, EPS_WCS))
//                {
//                    vec3 neighbourPoint = point.Equals(edge.first, EPS_WCS) ? edge.second : edge.first;
//                    bool sameExist = false;
//                    for (auto iter : neighbouringPoints)
//                    {
//                        if (iter.Equals(neighbourPoint, EPS_NORMAL))
//                        {
//                            sameExist = true;
//                            break;
//                        }
//                    }
//                    if (!sameExist)
//                    {
//                        neighbouringPoints.push_back(neighbourPoint);
//                    }
//                }
//            }
//        }
//        std::vector<CSGPlane> concavityCheckPlanes;
//        if (neighbouringPoints.size() >= 3)
//        {
//            for (int i = 0; i < neighbouringPoints.size() - 2; ++i)
//            {
//                for (int j = i + 1; j < neighbouringPoints.size() - 1; ++j)
//                {
//                    for (int k = j + 1; k < neighbouringPoints.size(); ++k)
//                    {
//                        vec3 crossProduct = Normalize(Cross(neighbouringPoints[j] - neighbouringPoints[i], neighbouringPoints[k] - neighbouringPoints[i]));
//                        CSGPlane intersectionCheckingPlane;
//                        intersectionCheckingPlane.n = crossProduct;
//                        intersectionCheckingPlane.d = Dot(neighbouringPoints[i], crossProduct);
//                        if (Dot(vecToNewPointNormalized, intersectionCheckingPlane.n) < 0.f)
//                        {
//                            intersectionCheckingPlane.n = -intersectionCheckingPlane.n;
//                            intersectionCheckingPlane.d = -intersectionCheckingPlane.d;
//                        }
//                        AddPlaneToUniqueSetOfPlanes(&potentialIntersectingPlanes, intersectionCheckingPlane, 0.001f);// shouldn't add two same ones?
//                    }
//                }
//            }
//        }
//    }

    
    // SANITIZE

    std::vector<vec3> snappedPointCloud;
    snappedPointCloud.reserve(brush->pointCloud.size());
    for (int i = 0; i < brush->pointCloud.size(); ++i)
    {
        vec3 snappedPoint;
        vec3 realPoint = brush->pointCloud[i];

        // 2022-11-01 Kevin - Initially I wanted to snap points to the grid but I see now that
        // just rounding them to the nearest integer is enough to eliminate floating point imprecision
        // issues.

        // Round to nearest integer
        snappedPoint.x = float(int(realPoint.x + 0.5f - (realPoint.x < 0.f)));
        snappedPoint.y = float(int(realPoint.y + 0.5f - (realPoint.y < 0.f)));
        snappedPoint.z = float(int(realPoint.z + 0.5f - (realPoint.z < 0.f)));

        snappedPointCloud.push_back(snappedPoint);

        if (realPoint.Equals(*ye, 0.000005f))
        {
            *ye = snappedPoint;
        }
    }
    for (int hn = 0; hn < brush->halfspaces.size(); ++hn)
    {
        std::vector<int> hsVertexIndices = brush->planeToVertices[hn];
        if (hsVertexIndices.size() < 3)
        {
            PrintLog.Error("there are less than 3 vertices on some plane...");
            continue;
        }
        vec3 a = snappedPointCloud[hsVertexIndices[0]];
        vec3 b = snappedPointCloud[hsVertexIndices[1]];
        vec3 c = snappedPointCloud[hsVertexIndices[2]];
        vec3 ba = b - a;
        vec3 ca = c - a;
        vec3 crossProduct = Normalize(Cross(ba, ca));
        CSGPlane snappedHalfspacePlane;
        snappedHalfspacePlane.n = crossProduct;
        snappedHalfspacePlane.d = Dot(a, crossProduct);
        if (Dot(brush->halfspaces[hn].n, snappedHalfspacePlane.n) < 0.f) // wrong direction as old plane
        {
            snappedHalfspacePlane.n = -snappedHalfspacePlane.n;
            snappedHalfspacePlane.d = -snappedHalfspacePlane.d;
        }
        brush->halfspaces[hn] = snappedHalfspacePlane;
    }
    brush->pointCloud = snappedPointCloud;


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
            std::vector<vec3> commonVertices;
            BrushPlane_FindCommonVertices(brush, i, j, &commonVertices);

            if (commonVertices.size() > 1)
            {
                for (int commonI = 0; commonI < commonVertices.size() - 1; ++commonI)
                {
                    for (int commonJ = commonI + 1; commonJ < commonVertices.size(); ++commonJ)
                    {
                        int cei = -1;
                        int cej = -1;
                        BrushPoint_FindIndexInBrush(&brush->pointCloud, commonVertices[commonI], &cei, 0.f);
                        BrushPoint_FindIndexInBrush(&brush->pointCloud, commonVertices[commonJ], &cej, 0.f);
                        std::pair<int, int> edge = std::make_pair(cei, cej);
                        brush->planeToEdges[i].push_back(edge);
                        brush->planeToEdges[j].push_back(edge);
                        brush->edgesSet.push_back(edge);
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
        std::vector<int> vertsIndices = brush->planeToVertices[i];
        vec3 pickedVert = brush->pointCloud[vertsIndices[0]];

        LevelGeometry::FaceBatch face;
        LevelGeometry::CreateFaceBatchMeshOnGPU(&face.idVAO, &face.idVBO);

        std::vector<float> vertexBuffer;

        for (int e = 0; e < brush->planeToEdges[i].size(); ++e)
        {
            std::pair<int, int> edgeVertexIndices = brush->planeToEdges[i][e];
            std::pair<vec3, vec3> edge = std::make_pair(brush->pointCloud[edgeVertexIndices.first], brush->pointCloud[edgeVertexIndices.second]);
            if (!pickedVert.Equals(edge.first, EPS_WCS) && !pickedVert.Equals(edge.second, EPS_WCS))
            {
                float uvScale = (1.f / 32.f);

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

bool BrushContainsPoint(CSGBrush* brush, vec3 point, float eps)
{
    for (int c = 0; c < brush->pointCloud.size(); ++c)
    {
        if (brush->pointCloud[c].Equals(point, eps))
        {
            return true;
        }
    }
    return false;
}

CSGBrush* DoSmth()
{
// planes data
    CSGPlane p0;
    p0.n = Normalize(vec3(1, 0, 0));
    p0.d = 16.f;
    CSGPlane p1;
    p1.n = Normalize(vec3(0, 1, 0));
    p1.d = 16.f;
    CSGPlane p2;
    p2.n = Normalize(vec3(0, 0, 1));
    p2.d = 16.f;
    CSGPlane p3;
    p3.n = Normalize(vec3(-1, 0, 0));
    p3.d = 16.f;
    CSGPlane p4;
    p4.n = Normalize(vec3(0, -1, 0));
    p4.d = 16.f;
    CSGPlane p5;
    p5.n = Normalize(vec3(0, 0, -1));
    p5.d = 16.f;

    CSGPlane slice;
    slice.n = Normalize(vec3(1,1,0));
    slice.d = 16.f;

    CSGBrush* brush = new CSGBrush();
    brush->halfspaces.push_back(p0);
    brush->halfspaces.push_back(p1);
    brush->halfspaces.push_back(p2);
    brush->halfspaces.push_back(p3);
    brush->halfspaces.push_back(p4);
    brush->halfspaces.push_back(p5);
    //brush->halfspaces.push_back(slice);

    vec3 temp = vec3();
    UpdateBrushVerticesEdgesDataFromHalfSpaces(brush, &temp);
    UpdateBrushFacesForDrawing(brush);

    return brush;
}
