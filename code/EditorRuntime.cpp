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

// brush visual data is bound to gpu on creation -> take vao vbo id
// vao vbo is updated when brush is modified

// TODO(Kevin): probably need optimization like face batching or smth eventually


static GameCamera editorCamera;

void EditorRuntime::StartEditor()
{
    Engine.gfx->activeGameCamera = &editorCamera;
    editorCamera.position.x = -512.f;
    
    brush_0 = DoSmth();
}

void EditorRuntime::UpdateEditor()
{
	i32 drawableWidth;
	i32 drawableHeight;
	Engine.gfx->GetBackBufferSize(&drawableWidth, &drawableHeight);

    KevGui::EditorBeginWindow(KevGui::UIRect(drawableWidth - 300, 0, 300, drawableHeight));
    if (KevGui::EditorLabelledButton("A"))
    {
        printf("A");
    }
    if (KevGui::EditorLabelledButton("Longer Button B"))
    {
        printf("B");
    }
    KevGui::EditorIncrementableIntegerField("Integer", &num);
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
    float moveSpeed = 512.f;
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
    for (int i = 0; i < brush_0->pointCloud.size(); ++i)
    {
        auto v = brush_0->pointCloud[i];
        Handles::DrawSolidDisc(v, -editorCamera.calculatedDirection, Handles::GetHandleSize(v, 5.f));

        if (!handleDragActive)
        {
            // Find nearest control
            float vToMouseDist = Handles::DistanceToCircle(v, Handles::GetHandleSize(v, 5.f));
            float vToCamDist = editorCamera.WorldPointToCameraDistance(v);
            if (vToMouseDist == 0.f && vToCamDist < selectedPointToCamDistance)
            {
                pointSelectedThisFrame = v;
                selectedPointToCamDistance = vToCamDist;
            }
        }
    }

    if (Input.mouseLeftHasBeenPressed && selectedPointToCamDistance < FLT_MAX)
    {
        handleDragActive = true;
        selectedDragPoint = pointSelectedThisFrame;
    }

    if (handleDragActive && Input.mouseLeftHasBeenReleased)
    {
        handleDragActive = false;
    }

    if (handleDragActive)
    {
        auto v = selectedDragPoint;
        char charbuf[32];
        stbsp_sprintf(charbuf, "%.1f, %.1f, %.1f", v.x, v.y, v.z);
        printf("%s\n", charbuf);

        Handles::DrawLine(v + vec3(1000.f, 0.f, 0.f), v + vec3(-1000.f, 0.f, 0.f), vec4(_RGB(248, 230, 60), 0.7f));
        Handles::DrawLine(v + vec3(0.f, 1000.f, 0.f), v + vec3(0.f, -1000.f, 0.f), vec4(_RGB(248, 230, 60), 0.7f));
        Handles::DrawLine(v + vec3(0.f, 0.f, 1000.f), v + vec3(0.f, 0.f, -1000.f), vec4(_RGB(248, 230, 60), 0.7f));

        if (Input.mouseLeftPressed)
        {
            vec3 dragOrigin = selectedDragPoint;
            vec3 wp = editorCamera.ScreenToWorldPoint(vec3((float)Input.mousePos.x, (float)Input.mousePos.y, editorCamera.WorldPointToCameraDistance(dragOrigin)));
            vec3 intersection;

            if (Input.KeyPressed(SDL_SCANCODE_LALT))
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
                Handles::DrawLine(dragOrigin, dragOrigin + GM_UP_VECTOR * projY, 3.f, vec4(1, 0, 1, 1));

                if (GM_abs(projY) > (float)activeGridSize * 0.001f * 0.5f)
                {
                    vec3 movedPoint = selectedDragPoint + vec3(0.f, GM_sgn(projY) * (float)activeGridSize * 0.001f, 0.f);
                    MovePointOnBrushAndUpdateHalfSpaces(selectedDragPoint, movedPoint, brush_0, &movedPoint);
                    UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
                    UpdateBrushFacesForDrawing(brush_0);
                    selectedDragPoint = movedPoint;
                    if (!BrushContainsPoint(brush_0, selectedDragPoint))
                    {
                        handleDragActive = false;
                    }
                }
            }
            else
            {
                if (FindIntersectionBetweenPlaneAndVector(GM_UP_VECTOR, dragOrigin.y, Normalize(wp - editorCamera.position), editorCamera.position, &intersection))
                {
                    float projX = Dot(intersection - dragOrigin, GM_FORWARD_VECTOR);
                    Handles::DrawLine(dragOrigin, dragOrigin + GM_FORWARD_VECTOR * projX, 3.f, vec4(1, 0, 1, 1));
                    float projZ = Dot(intersection - dragOrigin, GM_RIGHT_VECTOR);
                    Handles::DrawLine(dragOrigin, dragOrigin + GM_RIGHT_VECTOR * projZ, 3.f, vec4(1, 0, 1, 1));

                    bool moveX = GM_abs(projX) > (float)activeGridSize * 0.001f * 0.5f;
                    bool moveZ = GM_abs(projZ) > (float)activeGridSize * 0.001f * 0.5f;

                    if (moveX || moveZ)
                    {
                        vec3 movedPoint = selectedDragPoint + vec3(
                            moveX ? GM_sgn(projX) * (float)activeGridSize * 0.001f : 0.f,
                            0.f,
                            moveZ ? GM_sgn(projZ) * (float)activeGridSize * 0.001f : 0.f);
                        MovePointOnBrushAndUpdateHalfSpaces(selectedDragPoint, movedPoint, brush_0, &movedPoint);
                        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
                        UpdateBrushFacesForDrawing(brush_0);
                        selectedDragPoint = movedPoint;
                        if (!BrushContainsPoint(brush_0, selectedDragPoint))
                        {
                            handleDragActive = false;
                        }
                    }
                }
            }
        }
    }
}
