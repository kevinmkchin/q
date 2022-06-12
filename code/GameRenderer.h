#pragma once

#include "Common.h"
#include "Core.h"
#include "Shader.h"


struct BasicFrameBuffer
{
	u32 FBO;
	u32 colorTexId;
	u32 depthRBO;
	i32 width;
	i32 height;
};

class GameRenderer
{
public:
	bool Init();

	void Render();

	//void UpdateScreenSize();

private:

	void RenderGameLayer();

	void RenderGUILayer();

	void RenderDebugUILayer();

	void FinalRenderToBackBuffer();

private:

	void CreateFrameBuffers();

	void UpdateFrameBuffersSize();

	void CreateBasicFrameBuffer(BasicFrameBuffer* buffer);

	void UpdateBasicFrameBufferSize(BasicFrameBuffer* buffer);

	void CreateMiscellaneous();

private:

	i32 backBufferWidth;
	i32 backBufferHeight;

	BasicFrameBuffer gameLayer;
	BasicFrameBuffer guiLayer;
	BasicFrameBuffer debugUILayer;

	Shader finalPassShader;

private:
	Mesh screenSizeQuad;

};
