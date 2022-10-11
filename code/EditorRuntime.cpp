#include "EditorRuntime.h"

#include <gmath.h>

#include "Common.h"
#include "Engine.h"
#include "GameRenderer.h"
#include "GUI.h"

int num = 10;
float numf = 6.4f;

void EditorRuntime::UpdateEditor()
{
	KevGui::DoText(25, 40, 16, KevGui::TextAlignment::Left, "EDITOR");

	i32 drawableWidth;
	i32 drawableHeight;
	Engine.gfx->GetBackBufferSize(&drawableWidth, &drawableHeight);
	KevGui::DoPanel(KevGui::UIRect(drawableWidth - 300, 0, 300, drawableHeight), vec4(0.1f, 0.1f, 0.1f, 1.f));

	KevGui::DoImage(KevGui::UIRect(drawableWidth - 280, 40, 250, 250), Resources.defaultMissingTexture.textureId);

    KevGui::UIStyle style1 = KevGui::GetActiveUIStyleCopy();
    style1.textFont = Resources.fonts[FONT_SourceCodePro];
    KevGui::PushUIStyle(style1);
    KevGui::LabelledButton(KevGui::UIRect(drawableWidth - 280, 300, 250, 40), "Click qpy", 24, KevGui::TextAlignment::Center);
    KevGui::PopUIStyle();


	KevGui::DoIntegerInputField(KevGui::FreshID(), KevGui::UIRect(10, 300, 100, 25), &num);

    KevGui::DoFloatInputField(KevGui::FreshID(), KevGui::UIRect(10, 340, 100, 25), &numf);

    KevGui::EditorBeginWindow(KevGui::UIRect(200, 200, 200, 200));
    if(KevGui::EditorLabelledButton("A"))
    {
        printf("A");
    }
    if(KevGui::EditorLabelledButton("Longer Button B"))
    {
        printf("B");
    }
    KevGui::EditorIncrementableIntegerField("Some Integer", &num);
    KevGui::EditorIncrementableFloatField("Some Float", &numf);
    KevGui::EditorEndWindow();
}
