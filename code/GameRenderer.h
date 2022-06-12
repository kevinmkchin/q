#pragma once

#include "Common.h"

class GameRenderer
{
public:
	bool Init();

	void Render();

private:

	void RenderGameLayer();

	void RenderGUILayer();

	void RenderDebugUILayer();

	void FinalRenderToBackBuffer();

private:

	void CreateFrameBuffers();

	void UpdateFrameBuffersSize();

private:



};
