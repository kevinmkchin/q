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
    if(Input.mouseRMBPressed)
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
    if (Input.IsKeyPressed(SDL_SCANCODE_W))
        camPos += Engine.gfx->activeGameCamera->calculatedDirection * moveSpeed * Time.deltaTime;
    if (Input.IsKeyPressed(SDL_SCANCODE_A))
        camPos += -Engine.gfx->activeGameCamera->calculatedRight * moveSpeed * Time.deltaTime;
    if (Input.IsKeyPressed(SDL_SCANCODE_S))
        camPos += -Engine.gfx->activeGameCamera->calculatedDirection * moveSpeed * Time.deltaTime;
    if (Input.IsKeyPressed(SDL_SCANCODE_D))
        camPos += Engine.gfx->activeGameCamera->calculatedRight * moveSpeed * Time.deltaTime;
    if (Input.IsKeyPressed(SDL_SCANCODE_Q))
        camPos.y += -moveSpeed * Time.deltaTime;
    if (Input.IsKeyPressed(SDL_SCANCODE_E))
        camPos.y += moveSpeed * Time.deltaTime;
    editorCamera.CameraUpdate(camPos, mouseLook.x, mouseLook.y);
    editorCamera.CameraCalculatePerspectiveMatrix();


    int selectedPointIndex = -1;
    float selectedPointToCamDistance = FLT_MAX;
    static vec3 yolo = vec3(-128, 128, 128);
    for (int i = 0; i < brush_0->pointCloud.size(); ++i)
    {
        auto v = brush_0->pointCloud[i];
        Handles::DrawSolidDisc(v, -editorCamera.calculatedDirection, Handles::GetHandleSize(v, 5.f));
        // Find nearest control
        float vToMouseDist = Handles::DistanceToCircle(v, Handles::GetHandleSize(v, 5.f));
        float vToCamDist = editorCamera.WorldPointToCameraDistance(v);
        if (vToMouseDist == 0.f && vToCamDist < selectedPointToCamDistance)
        {
            selectedPointIndex = i;
            selectedPointToCamDistance = vToCamDist;
        }
    }

    if (Input.mouseLMBPressed && selectedPointIndex >= 0)
    {
        auto v = brush_0->pointCloud[selectedPointIndex];
        char charbuf[32];
        stbsp_sprintf(charbuf, "%.1f, %.1f, %.1f", v.x, v.y, v.z);
        printf("%s\n", charbuf);
        yolo = v;
    }


    static std::vector<CSGPlane> previousHalfspaces;
    static vec3 prevYolo = vec3();
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_I))
    {
        previousHalfspaces = brush_0->halfspaces;
        prevYolo = yolo;
        vec3 newYolo = yolo + vec3(10, 0, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_K))
    {
        previousHalfspaces = brush_0->halfspaces;
        prevYolo = yolo;
        vec3 newYolo = yolo + vec3(-10, 0, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_J))
    {
        previousHalfspaces = brush_0->halfspaces;
        prevYolo = yolo;
        vec3 newYolo = yolo + vec3(0, 0, -10);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_L))
    {
        previousHalfspaces = brush_0->halfspaces;
        prevYolo = yolo;
        vec3 newYolo = yolo + vec3(0, 0, 10);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_U))
    {
        previousHalfspaces = brush_0->halfspaces;
        prevYolo = yolo;
        vec3 newYolo = yolo + vec3(0, 10, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_O))
    {
        previousHalfspaces = brush_0->halfspaces;
        prevYolo = yolo;
        vec3 newYolo = yolo + vec3(0, -10, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if (Input.HasKeyBeenPressed(SDL_SCANCODE_M))
    {
        brush_0->halfspaces = previousHalfspaces;
        yolo = prevYolo;
        brush_0->pointCloud.clear();
        brush_0->pointToPlanes.clear();
        brush_0->planeToVertices.clear();
        brush_0->planeToEdges.clear();
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
    }

}
