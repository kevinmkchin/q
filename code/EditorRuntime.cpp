#include "EditorRuntime.h"

#include <gmath.h>

#include "Common.h"
#include "Engine.h"
#include "GameRenderer.h"
#include "GUI.h"

int num = 10;

void EditorRuntime::UpdateEditor()
{
	KevGui::DoText(25, 40, 16, KevGui::TextAlignment::Left, "EDITOR");

	i32 drawableWidth;
	i32 drawableHeight;
	Engine.gfx->GetBackBufferSize(&drawableWidth, &drawableHeight);
	KevGui::DoPanel(KevGui::UIRect(drawableWidth - 300, 0, 300, drawableHeight), vec4(0.1f, 0.1f, 0.1f, 1.f));

	KevGui::DoImage(KevGui::UIRect(drawableWidth - 280, 40, 250, 250), Resources.defaultMissingTexture.textureId);

    KevGui::LabelledButton(KevGui::UIRect(drawableWidth - 280, 300, 250, 40), "Click qpy", 24, KevGui::TextAlignment::Center);

    KevGui::IncrementableIntegerField("Some Integer", &num);

	KevGui::DoIntegerField(KevGui::FreshID(), KevGui::UIRect(300, 300, 100, 25), &num);

    KevGui::EditorBeginWindow(KevGui::UIRect(200, 200, 200, 200));
    if(KevGui::EditorLabelledButton("A"))
    {
        printf("A");
    }
    if(KevGui::EditorLabelledButton("B"))
    {
        printf("B");
    }
    KevGui::EditorEndWindow();
}
