#include "EditorRuntime.h"

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
    KevGui::EditorEndWindow();
    
    
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
 
    if(Input.HasKeyBeenPressed(SDL_SCANCODE_H))
    {
        static vec3 yolo = vec3(-100, 100, 100);
        vec3 newYolo = yolo + vec3(40, 0, 0);
        MovePointOnBrushAndUpdateHalfSpaces(yolo, newYolo, brush_0);
        UpdateBrushVerticesEdgesDataFromHalfSpaces(brush_0);
        UpdateBrushFacesForDrawing(brush_0);
        yolo = newYolo;
    }

}
