#include "ConstructiveSolidGeometry.h"

#include <unordered_map>
#include <algorithm>

#include "Utils.h"

static bool AddUniqueValidHalfspaceToBrush(std::vector<CSGPlane>* hsptr, CSGPlane p, float eps)
{
    if (p.n.Equals(vec3(0.f, 0.f, 0.f), 0.f)) return false;
    bool alreadyExist = false;
    for (auto iter : *hsptr)
        if (iter.Equals(p, eps))
            alreadyExist = true;
    if (!alreadyExist) hsptr->push_back(p);
    return !alreadyExist;
}

static bool RemoveHalfspaceFromBrush(std::vector<CSGPlane>* hsptr, CSGPlane p, float eps)
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

static void BrushPlane_FindIndexInBrush(std::vector<CSGPlane>* brushPlanes, 
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

    if (Magnitude(newPoint - point) < 0.98f) return;

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

    std::vector<CSGPlane> existingPlanes = brush->halfspaces;
    std::vector<CSGPlane> imaginaryPlanes;

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
                    if (Dot(vecToNewPointNormalized, intersectionCheckingPlane.n) < 0.f) // wrong direction as old plane
                    {
                        intersectionCheckingPlane.n = -intersectionCheckingPlane.n;
                        intersectionCheckingPlane.d = -intersectionCheckingPlane.d;
                    }
                    AddUniqueValidHalfspaceToBrush(&imaginaryPlanes, intersectionCheckingPlane, 0.001f);// shouldn't add two same ones?
                }
            }
        }
    }


    // CHECK for planes of intersection?
    // what if theres more than one plane of intersection?

    // if intersect w imaginary plane, remove all neighbour planes if num points on plane is 3
    // OR if more than 3 points, then remove if fold out movement ??
    // if fold in movement then 

    NiceArray<CSGPlane, 6> _intersectedImaginaryPlanes;
    NiceArray<vec3, 6> _potentialPointsOfIntersection;
    NiceArray<float, 6> _intersectionPointDistanceToOriginalPoint;
    for (auto imaginaryPlane : imaginaryPlanes)
    {
        if (!IsPointContainedInHalfSpace(imaginaryPlane.n, imaginaryPlane.d, point, 0.f)) continue;
        vec3 l0 = point;
        vec3 l = vecToNewPointNormalized;
        float ldotn = Dot(l, imaginaryPlane.n);
        vec3 pointOfIntersection;
        if (ldotn > 0.001f && FindIntersectionBetweenPlaneAndVector(imaginaryPlane.n, imaginaryPlane.d, l, l0, &pointOfIntersection))
        {
            float distanceToPointOfIntersection = Magnitude(pointOfIntersection - point);
            if (!pointOfIntersection.Equals(point, EPS_WCS) && distanceToPointOfIntersection <= Magnitude(vecToNewPoint) + 0.01f)
            {
                _intersectedImaginaryPlanes.PushBack(imaginaryPlane);
                _potentialPointsOfIntersection.PushBack(pointOfIntersection);
                _intersectionPointDistanceToOriginalPoint.PushBack(distanceToPointOfIntersection);
            }
        }
    }
    // When intersect w multiple imaginary planes (given that the intersecting point 
    // is closer than the point we want to move to), and the distance to more than one 
    // imaginary plane are the same (e.g. if two intersecting imaginary planes are the 
    // same distance away from the original point), then add multiple imaginary planes 
    // as real planes.
    NiceArray<CSGPlane, 6> closestImaginaryPlanes;
    float _shortestDistanceAwaySoFar = FLT_MAX;
    for (int i = 0; i < _intersectedImaginaryPlanes.count; ++i)
    {
        float iterDist = _intersectionPointDistanceToOriginalPoint.At(i);
        if (iterDist - 0.01f <= _shortestDistanceAwaySoFar)
        {
            if (iterDist + 0.01f < _shortestDistanceAwaySoFar)
            {
                closestImaginaryPlanes.ResetCount();
                _shortestDistanceAwaySoFar = iterDist;

                newPoint = _potentialPointsOfIntersection.At(i);
                *ye = newPoint;
            }
            closestImaginaryPlanes.PushBack(_intersectedImaginaryPlanes.At(i));
        }
    }
    if (closestImaginaryPlanes.count > 0)
    {
        std::vector<CSGPlane> toRemove;
        for (auto afp : affectedFacePlanes)
        {
            int indexOfAffectedPlane = -1;
            BrushPlane_FindIndexInBrush(&existingPlanes, afp, &indexOfAffectedPlane, 0.001f);
            ASSERT(indexOfAffectedPlane >= 0);
            auto faceVertices = brush->planeToVertices[indexOfAffectedPlane];
            if (faceVertices.size() == 3)
            {
                // check if this triangle shares two points (other than the moved point)
                // with this intersected imaginary plane
                // perform this check by checking IsPointOnPlane with imaginary plane and the points of
                // the triangles we want to remove -> if at least 2 points of a triangle are on this imaginary plane
                // then that triangle can fuck off
                for (int i = 0; i < closestImaginaryPlanes.count; ++i)
                {
                    CSGPlane imaginaryPlane = closestImaginaryPlanes.At(i);
                    int numTriangleVerticesOnImaginaryPlane = 0;
                    for (auto vi : faceVertices)
                    {
                        vec3 v = brush->pointCloud[vi];
                        if (IsPointOnPlane(imaginaryPlane.n, imaginaryPlane.d, v, 0.001f))
                        {
                            if (++numTriangleVerticesOnImaginaryPlane == 2) break;
                        }
                    }
                    if (numTriangleVerticesOnImaginaryPlane >= 2)
                    {
                        toRemove.push_back(afp);
                        break;
                    }
                }
            }
        }
        printf("added %d planes\n", closestImaginaryPlanes.count);
        printf("removed %zd planes\n", toRemove.size());
        for (auto ri : toRemove)
        {
            RemoveHalfspaceFromBrush(&brush->halfspaces, ri, 0.f);
            RemoveHalfspaceFromBrush(&affectedFacePlanes, ri, 0.f);
        }
        for (int i = 0; i < closestImaginaryPlanes.count; ++i)
        {
            CSGPlane halfspaceFormedFromMerge;
            int isImaginaryPlaneARealExistingPlane = -1;
            BrushPlane_FindIndexInBrush(&existingPlanes, closestImaginaryPlanes.At(i), &isImaginaryPlaneARealExistingPlane, 0.001f);
            if (isImaginaryPlaneARealExistingPlane < 0)
            {
                halfspaceFormedFromMerge.n = -closestImaginaryPlanes.At(i).n;
                halfspaceFormedFromMerge.d = -closestImaginaryPlanes.At(i).d;
            }
            AddUniqueValidHalfspaceToBrush(&brush->halfspaces, halfspaceFormedFromMerge, 0.00005f); // add the imaginary planes
        }
    }
    else // then check for intersection against existing real planes
    {
        for (auto existingPlane : existingPlanes)
        {
            vec3 l0 = point;
            vec3 l = vecToNewPointNormalized;
            float ldotn = Dot(l, existingPlane.n);
            vec3 pointOfIntersection;
            if (ldotn > 0.001f && FindIntersectionBetweenPlaneAndVector(existingPlane.n, existingPlane.d, l, l0, &pointOfIntersection))
            {
                if (!pointOfIntersection.Equals(point, EPS_WCS) && Magnitude(pointOfIntersection - point) <= Magnitude(vecToNewPoint) + 0.01f)
                {
                    newPoint = pointOfIntersection;
                    *ye = pointOfIntersection;

                    // for all relevant planes, if it has exactly 3 points then remove that plane from halfspaces and remove from relevant planes
                    std::vector<CSGPlane> toRemove;
                    for (auto afp : affectedFacePlanes)
                    {
                        int indexOfAffectedPlane = -1;
                        BrushPlane_FindIndexInBrush(&existingPlanes, afp, &indexOfAffectedPlane, 0.001f);
                        ASSERT(indexOfAffectedPlane >= 0);
                        auto faceVertices = brush->planeToVertices[indexOfAffectedPlane];
                        if (faceVertices.size() == 3)
                        {
                            // then check if this triangle shares two points (other than the moved point)
                            // with this intersected plane
                            int indexOfIntersectedPlane = -1;
                            BrushPlane_FindIndexInBrush(&existingPlanes, existingPlane, &indexOfIntersectedPlane, 0.001f);
                            std::vector<vec3> commonVertices;
                            BrushPlane_FindCommonVertices(brush, indexOfIntersectedPlane, indexOfAffectedPlane, &commonVertices);
                            if (commonVertices.size() >= 2)
                            {
                                toRemove.push_back(afp);
                            }
                        }
                    }
                    for (auto ri : toRemove)
                    {
                        RemoveHalfspaceFromBrush(&brush->halfspaces, ri, 0.f);
                        RemoveHalfspaceFromBrush(&affectedFacePlanes, ri, 0.f);
                    }

                    break;
                }
            }
        }
    }



    // at this point, we want to make sure EVERY plane we try to add to the brush doesn't already exist...
    for (auto plane : affectedFacePlanes)
    {
        if (IsPointOnPlane(plane.n, plane.d, newPoint, 0.01f)) continue; // no change

        int indexOfPlane = -1;
        BrushPlane_FindIndexInBrush(&existingPlanes, plane, &indexOfPlane, 0.001f);

        if (brush->planeToVertices[indexOfPlane].size() > 3) // face has more than 3 points
        {
            if (IsPointContainedInHalfSpace(plane.n, plane.d, newPoint, 0.01f))
            {
                // fold in
                // simply add new plane formed from newPoint and the immediate neighbours of old point
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
                AddUniqueValidHalfspaceToBrush(&brush->halfspaces, createdPlane, 0.00005f); // TODO(Kevin): revisit epsilon
            }
            else
            {
                // fold out
                // remove og plane from halfspaces, then add plane formed from new point and every non neighbour edge of old point
                RemoveHalfspaceFromBrush(&brush->halfspaces, plane, 0.f);

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
                        AddUniqueValidHalfspaceToBrush(&brush->halfspaces, createdPlane, 0.00005f);
                    }
                }
            }
        }
        else if (brush->planeToVertices[indexOfPlane].size() == 3)
        {
            // when planeToVertices and subsequently planeToEdges gets fucked up
            // then we can't find a value for b or c for some planes and we get fucked

            // remove og plane, then add plane formed from newPoint and the immediate neighbours of old point
            RemoveHalfspaceFromBrush(&brush->halfspaces, plane, 0.f);
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
            AddUniqueValidHalfspaceToBrush(&brush->halfspaces, createdPlane, 0.00005f); // TODO(Kevin): revisit epsilon
        }

    }

    // we know that, as long as brush->halfspaces is correct, all subsequent vectors and calculations will be correct
}

void UpdateBrushVerticesEdgesDataFromHalfSpaces(CSGBrush* brush)
{
    brush->pointCloud.clear();
    brush->pointToPlanes.clear();
    brush->planeToVertices.clear();
    brush->planeToEdges.clear();

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

                if (GM_abs(t) <= 0.0001f) continue; // the planes are parallel

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
                            int indexOfPointInCloud = brush->pointCloud.size() - 1;
                            brush->pointToPlanes[indexOfPointInCloud].push_back(p);
                            brush->planeToVertices[p].push_back(indexOfPointInCloud);
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
                        brush->planeToEdges[i].push_back(std::make_pair(cei, cej));
                        brush->planeToEdges[j].push_back(std::make_pair(cei, cej));
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
    //brush->halfspaces.push_back(slice);

    UpdateBrushVerticesEdgesDataFromHalfSpaces(brush);
    UpdateBrushFacesForDrawing(brush);

    return brush;
}
