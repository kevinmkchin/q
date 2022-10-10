#pragma once

#include "Common.h"
#include "Resources.h"
#include "Shader.h"
#include "Camera.h"


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

	void UpdateBackBufferSize();

	void GetBackBufferSize(i32* widthOutput, i32* heightOutput);

private:

	void RenderGameLayer();

	void RenderGUILayer();

	void RenderDebugUILayer();

	void FinalRenderToBackBuffer();

private:

	void CreateFrameBuffers();

	void UpdateFrameBuffersSize();

	void CreateBasicFrameBuffer(BasicFrameBuffer* buffer);

	void UpdateBasicFrameBufferSize(BasicFrameBuffer* buffer, i32 newWidth, i32 newHeight);

	void CreateMiscellaneous();

private:
	i32 backBufferWidth = -1;
	i32 backBufferHeight = -1;

public:
	BasicFrameBuffer gameLayer;
	BasicFrameBuffer guiLayer;
	BasicFrameBuffer debugUILayer;

private:
	Shader finalPassShader;
	Shader gameSceneShader;

private:
	Mesh screenSizeQuad;

public:
	GameCamera* activeGameCamera = nullptr;

};
