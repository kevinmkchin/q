#include "EditorRuntime.h"

#include <gmath.h>

#include "Common.h"
#include "Engine.h"
#include "GameRenderer.h"
#include "GUI.h"

void EditorRuntime::UpdateEditor()
{
	KevGui::DoText("EDITOR", 25, 40, 16);

	i32 drawableWidth;
	i32 drawableHeight;
	Engine.gfx->GetBackBufferSize(&drawableWidth, &drawableHeight);
	KevGui::DoPanel(KevGui::UIRect(drawableWidth - 300, 0, 300, drawableHeight), vec4(0.1f, 0.1f, 0.1f, 1.f));

}
