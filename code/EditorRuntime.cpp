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

	KevGui::DoImage(KevGui::UIRect(40, drawableHeight - 290, 250, 250), Resources.defaultMissingTexture.textureId);

    KevGui::UIStyle style1 = KevGui::GetActiveUIStyleCopy();
    style1.textFont = Resources.fonts[FONT_SourceCodePro];
    KevGui::PushUIStyle(style1);
    KevGui::LabelledButton(KevGui::UIRect(40, drawableHeight - 340, 250, 40), "Click me!", 24, KevGui::TextAlignment::Center);
    KevGui::PopUIStyle();
    KevGui::LabelledButton(KevGui::UIRect(300, drawableHeight - 340, 250, 40), "Click me!", 24, KevGui::TextAlignment::Center);

	KevGui::DoIntegerInputField(KevGui::FreshID(), KevGui::UIRect(10, 300, 100, 25), &num);
    KevGui::DoFloatInputField(KevGui::FreshID(), KevGui::UIRect(2, 310, 100, 25), &numf);
}
