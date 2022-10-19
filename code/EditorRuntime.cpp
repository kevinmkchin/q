#include "EditorRuntime.h"

#include <stb_sprintf.h>
#include <gmath.h>

#include "Common.h"
#include "Engine.h"
#include "GameRenderer.h"
#include "GUI.h"
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


    vec2 mouseLook = vec2();
    if(Input.mouseRMBPressed)
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        mouseLook = vec2(Input.mouseDelta.x * 1.7f, Input.mouseDelta.y * 1.7f);
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

    static vec3 yolo = vec3(-100, 100, 100);
    for (auto v : brush_0->pointCloud)
    {
        char charbuf[32];
        stbsp_sprintf(charbuf, "%.1f, %.1f, %.1f", v.x, v.y, v.z);
        if(KevGui::EditorLabelledButton(charbuf))
        {
            yolo = v;
        }
    }
    KevGui::EditorEndWindow();
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_I))
    {
        vec3 newYolo = yolo + vec3(10, 0, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_K))
    {
        vec3 newYolo = yolo + vec3(-10, 0, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_J))
    {
        vec3 newYolo = yolo + vec3(0, 0, -10);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_L))
    {
        vec3 newYolo = yolo + vec3(0, 0, 10);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_U))
    {
        vec3 newYolo = yolo + vec3(0, 10, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_O))
    {
        vec3 newYolo = yolo + vec3(0, -10, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0, &newYolo);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }

}
