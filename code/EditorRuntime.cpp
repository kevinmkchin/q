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
    
    brush_0 = DoSmth();
}

void EditorRuntime::UpdateEditor()
{
	KevGui::DoText(25, 40, 16, KevGui::TextAlignment::Left, "EDITOR");

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
    
    if(Input.mouseRMBPressed)
    {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        vec3 camPos = editorCamera.position;
        float moveSpeed = 100.f;
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
        editorCamera.CameraUpdate(camPos, Input.mouseDelta.x * 1.7f, Input.mouseDelta.y * 1.7f);
        editorCamera.CameraCalculatePerspectiveMatrix();
    }
    else
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
    
}
