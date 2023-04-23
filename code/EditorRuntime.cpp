#include "EditorRuntime.h"

#include <stb_sprintf.h>
#include <gmath.h>

#include "Common.h"
#include "Engine.h"
#include "GameRenderer.h"
#include "GUI.h"
#include "HandleUtility.h"
#include "Input.h"
#include "ConstructiveSolidGeometry.h"

int num = 10;
float numf = 6.4f;


// TODO(Kevin): probably need optimization like face batching or smth eventually

static GameCamera editorCamera;

static std::vector<CSGBrush*> s_selectedBrushes;
static std::vector<vec3> s_movingPointsOfBrush;

// list of vec3 -> average vec3
static vec3 AverageVector(const std::vector<vec3>* vec3s)
{
    vec3 sum = vec3();
    for (vec3 v : *vec3s)
    {
        sum += v;
    }
    sum /= float(vec3s->size());
    return sum;
}

// -> null
static void MovePoint(CSGBrush* selectedBrush, vec3* movingPoint, vec3 dragOrigin, EditorGridSize gridSize, bool yAxisActive)
{
    vec3 wp = editorCamera.ScreenToWorldPoint(vec3((float)Input.mousePos.x, (float)Input.mousePos.y, editorCamera.WorldPointToCameraDistance(dragOrigin)));
    vec3 intersection;

    if (yAxisActive)
    {
        float projY = 0.f;
        if (GM_abs(Dot(editorCamera.calculatedDirection, GM_FORWARD_VECTOR)) > 0.707)
        {
            if (FindIntersectionBetweenPlaneAndVector(GM_FORWARD_VECTOR, dragOrigin.x, Normalize(wp - editorCamera.position), editorCamera.position, &intersection))
            {
                projY = Dot(intersection - dragOrigin, GM_UP_VECTOR);
            }
        }
        else
        {
            if (FindIntersectionBetweenPlaneAndVector(GM_RIGHT_VECTOR, dragOrigin.z, Normalize(wp - editorCamera.position), editorCamera.position, &intersection))
            {
                projY = Dot(intersection - dragOrigin, GM_UP_VECTOR);
            }
        }

        if (GM_abs(projY) > (float)gridSize * 0.5f)
        {
            int projReadyToSnap = int(projY + GM_sgn(projY) * (float)gridSize * 0.5f);
            float moveDelta = float(projReadyToSnap - (projReadyToSnap % (int)gridSize));
            vec3 movedPoint = *movingPoint + vec3(0.f, moveDelta, 0.f);
            MovePointOnBrushAndUpdateHalfSpaces(*movingPoint, movedPoint, selectedBrush, &movedPoint);
            UpdateBrushVerticesEdgesDataFromHalfSpaces(selectedBrush, &movedPoint);
            UpdateBrushFacesForDrawing(selectedBrush);
            *movingPoint = movedPoint;
        }
    }
    else
    {
        if (FindIntersectionBetweenPlaneAndVector(GM_UP_VECTOR, dragOrigin.y, Normalize(wp - editorCamera.position), editorCamera.position, &intersection))
        {
            float projX = Dot(intersection - dragOrigin, GM_FORWARD_VECTOR);
            float projZ = Dot(intersection - dragOrigin, GM_RIGHT_VECTOR);

            bool moveX = GM_abs(projX) > (float)gridSize * 0.5f;
            bool moveZ = GM_abs(projZ) > (float)gridSize * 0.5f;

            if (moveX)
            {
                int projReadyToSnap = int(projX + GM_sgn(projX) * (float)gridSize * 0.5f);
                float moveDelta = float(projReadyToSnap - (projReadyToSnap % (int)gridSize));
                vec3 movedPoint = *movingPoint + vec3(moveDelta, 0.f, 0.f);
                MovePointOnBrushAndUpdateHalfSpaces(*movingPoint, movedPoint, selectedBrush, &movedPoint);
                UpdateBrushVerticesEdgesDataFromHalfSpaces(selectedBrush, &movedPoint);
                UpdateBrushFacesForDrawing(selectedBrush);
                *movingPoint = movedPoint;
            }
            if (moveZ)
            {
                int projReadyToSnap = int(projZ + GM_sgn(projZ) * (float)gridSize * 0.5f);
                float moveDelta = float(projReadyToSnap - (projReadyToSnap % (int)gridSize));
                vec3 movedPoint = *movingPoint + vec3(0.f, 0.f, moveDelta);
                MovePointOnBrushAndUpdateHalfSpaces(*movingPoint, movedPoint, selectedBrush, &movedPoint);
                UpdateBrushVerticesEdgesDataFromHalfSpaces(selectedBrush, &movedPoint);
                UpdateBrushFacesForDrawing(selectedBrush);
                *movingPoint = movedPoint;
            }
        }
    }
}

void EditorRuntime::StartEditor()
{
    Engine.gfx->activeGameCamera = &editorCamera;
    editorCamera.position.x = -128.f;
    
    brush_0 = DoSmth();

    s_selectedBrushes.push_back(brush_0);
}

void EditorRuntime::UpdateEditor()
{
	i32 drawableWidth;
	i32 drawableHeight;
	Engine.gfx->GetBackBufferSize(&drawableWidth, &drawableHeight);

    KevGui::EditorBeginWindow(KevGui::UIRect(drawableWidth - 300, 0, 300, drawableHeight));
    if (KevGui::EditorLabelledButton("Vertices"))
    {
        activeToolMode = EditorToolMode::EditVertex;
    }
    if (KevGui::EditorLabelledButton("Edges"))
    {
        activeToolMode = EditorToolMode::EditEdge;
    }
    if (KevGui::EditorLabelledButton("Faces"))
    {
        activeToolMode = EditorToolMode::EditFace;
    }
    KevGui::EditorIncrementableIntegerField("Grid", &num);
    KevGui::EditorIncrementableFloatField("Float", &numf);
    KevGui::EditorEndWindow();

    vec2 mouseLook = vec2();
    if(Input.mouseRightPressed)
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        mouseLook = vec2(Input.mouseDelta.x * 0.07f, Input.mouseDelta.y * 0.07f);
    }
    else
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    vec3 camPos = editorCamera.position;
    float moveSpeed = 128.f;
    if (Input.KeyPressed(SDL_SCANCODE_W))
        camPos += Engine.gfx->activeGameCamera->calculatedDirection * moveSpeed * Time.deltaTime;
    if (Input.KeyPressed(SDL_SCANCODE_A))
        camPos += -Engine.gfx->activeGameCamera->calculatedRight * moveSpeed * Time.deltaTime;
    if (Input.KeyPressed(SDL_SCANCODE_S))
        camPos += -Engine.gfx->activeGameCamera->calculatedDirection * moveSpeed * Time.deltaTime;
    if (Input.KeyPressed(SDL_SCANCODE_D))
        camPos += Engine.gfx->activeGameCamera->calculatedRight * moveSpeed * Time.deltaTime;
    if (Input.KeyPressed(SDL_SCANCODE_Q))
        camPos.y += -moveSpeed * Time.deltaTime;
    if (Input.KeyPressed(SDL_SCANCODE_E))
        camPos.y += moveSpeed * Time.deltaTime;
    editorCamera.CameraUpdate(camPos, mouseLook.x, mouseLook.y);
    editorCamera.CameraCalculatePerspectiveMatrix();

    Handles::DrawLine(vec3(10000.f, 0.f, 0.f), vec3(-10000.f, 0.f, 0.f), vec4(_RGB(205, 56, 9), 0.8f));
    Handles::DrawLine(vec3(0.f, 10000.f, 0.f), vec3(0.f, -10000.f, 0.f), vec4(_RGB(67, 123, 9), 0.8f));
    Handles::DrawLine(vec3(0.f, 0.f, 10000.f), vec3(0.f, 0.f, -10000.f), vec4(_RGB(21, 129, 205), 0.8f));


    vec3 pointSelectedThisFrame = vec3();
    float selectedPointToCamDistance = FLT_MAX;
    switch(activeToolMode)
    {
        case EditorToolMode::None:
            break;
        case EditorToolMode::EditVertex:
        {
            for (int i = 0; i < brush_0->pointCloud.size(); ++i)
            {
                auto v = brush_0->pointCloud[i];
                Handles::DrawSolidDisc(v, -editorCamera.calculatedDirection, Handles::GetHandleSize(v, 10.f));

                if (!handleDragActive)
                {
                    // Find nearest control
                    float vToMouseDist = Handles::DistanceToCircle(v, Handles::GetHandleSize(v, 10.f));
                    float vToCamDist = editorCamera.WorldPointToCameraDistance(v);
                    if (vToMouseDist == 0.f && vToCamDist < selectedPointToCamDistance)
                    {
                        pointSelectedThisFrame = v;
                        selectedPointToCamDistance = vToCamDist;
                    }
                }
            }
            for (int i = 0; i < brush_0->edgesSet.size(); ++i)
            {
                auto edge = brush_0->edgesSet[i];
                vec3 p1 = brush_0->pointCloud[edge.first];
                vec3 p2 = brush_0->pointCloud[edge.second];
                Handles::DrawLine(p1, p2, vec4(1.f, 0.f, 0.f, 1.f));
            }
        }break;
        case EditorToolMode::EditEdge:
        {
            for (int i = 0; i < brush_0->edgesSet.size(); ++i)
            {
                auto edge = brush_0->edgesSet[i];
                vec3 p1 = brush_0->pointCloud[edge.first];
                vec3 p2 = brush_0->pointCloud[edge.second];
                Handles::DrawLine(p1, p2, vec4(1.f, 1.f, 0.f, 1.f));
            }
            for (int i = 0; i < brush_0->edgesSet.size(); ++i)
            {
                auto edge = brush_0->edgesSet[i];
                vec3 p1 = brush_0->pointCloud[edge.first];
                vec3 p2 = brush_0->pointCloud[edge.second];
                vec3 midp = p1 + ((p2 - p1) / 2.f);
                Handles::DrawSolidDisc(midp, -editorCamera.calculatedDirection, Handles::GetHandleSize(midp, 10.f));
            }
        }break;
        case EditorToolMode::EditFace:
        {
            for (int i = 0; i < brush_0->edgesSet.size(); ++i)
            {
                auto edge = brush_0->edgesSet[i];
                vec3 p1 = brush_0->pointCloud[edge.first];
                vec3 p2 = brush_0->pointCloud[edge.second];
                Handles::DrawLine(p1, p2, vec4(1.f, 0.f, 0.f, 1.f));
            }
            for(int i = 0; i < brush_0->halfspaces.size(); ++i)
            {
                auto vertexIndices = brush_0->planeToVertices[i];
                vec3 avg;
                for (auto vi : vertexIndices)
                {
                    auto vert = brush_0->pointCloud[vi];
                    avg += vert;
                }
                avg = avg / float(vertexIndices.size());
                Handles::DrawSolidDisc(avg, -editorCamera.calculatedDirection, Handles::GetHandleSize(avg, 10.f));
            }
        }break;
    }



    if (Input.mouseLeftHasBeenPressed && selectedPointToCamDistance < FLT_MAX)
    {
        handleDragActive = true;
        handleDragPoint = pointSelectedThisFrame;

        s_movingPointsOfBrush.clear();
        s_movingPointsOfBrush.push_back(pointSelectedThisFrame);
    }
    
    if (handleDragActive)
    {
        handleDragPoint = AverageVector(&s_movingPointsOfBrush);
    }

    if (handleDragActive && Input.mouseLeftHasBeenReleased)
    {
        handleDragActive = false;
    }

    if (handleDragActive)
    {
        vec3 dragOrigin = handleDragPoint;
        //vec3 v = dragOrigin;
        //char charbuf[32];
        //stbsp_sprintf(charbuf, "%.1f, %.1f, %.1f", v.x, v.y, v.z);
        //printf("%s\n", charbuf);

        Handles::DrawLine(dragOrigin + vec3(1000.f, 0.f, 0.f), dragOrigin + vec3(-1000.f, 0.f, 0.f), vec4(_RGB(248, 230, 60), 0.7f));
        Handles::DrawLine(dragOrigin + vec3(0.f, 1000.f, 0.f), dragOrigin + vec3(0.f, -1000.f, 0.f), vec4(_RGB(248, 230, 60), 0.7f));
        Handles::DrawLine(dragOrigin + vec3(0.f, 0.f, 1000.f), dragOrigin + vec3(0.f, 0.f, -1000.f), vec4(_RGB(248, 230, 60), 0.7f));

        if (Input.mouseLeftPressed)
        {
            for (CSGBrush* selectedBrushIter : s_selectedBrushes)
            {
                for (vec3& movingPointRef : s_movingPointsOfBrush)
                {
                    vec3 movingPointMutable = movingPointRef;
                    if (BrushContainsPoint(selectedBrushIter, movingPointMutable, EPS_POINTINPOINTCLOUD))
                    {
                        MovePoint(selectedBrushIter, &movingPointMutable, dragOrigin, activeGridSize, Input.KeyPressed(SDL_SCANCODE_LALT));
                        if (!BrushContainsPoint(selectedBrushIter, movingPointMutable, EPS_POINTINPOINTCLOUD))
                        {
                            handleDragActive = false;
                        }
                        else
                        {
                            movingPointRef = movingPointMutable;
                        }
                    }
                }
            }
        }

    }
}
