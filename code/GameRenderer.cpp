#include "GameRenderer.h"

#include <glew.h>

#include "Common.h"

bool GameRenderer::Init()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        PrintLog.Error("GLEW failed to initialize.");
        return false;
    }
    PrintLog.Message("GLEW initialized.");

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // alpha blending func: a * (rgb) + (1 - a) * (rgb) = final color output
    glBlendEquation(GL_FUNC_ADD);

    backBufferWidth = 800;
    backBufferHeight = 600;

    CreateFrameBuffers();
    UpdateFrameBuffersSize();

    Shader::GLLoadShaderProgramFromFile(finalPassShader, shader_path("finalpass.vert").c_str(), shader_path("finalpass.frag").c_str());

    CreateMiscellaneous();

    return true;
}

void GameRenderer::Render()
{
    RenderGameLayer();
    RenderGUILayer();
    RenderDebugUILayer();

    FinalRenderToBackBuffer();
}

void GameRenderer::RenderGameLayer()
{
    glViewport(0, 0, 800, 600);
    glClearColor(_RGB(46, 88, 120), 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void GameRenderer::RenderGUILayer()
{

}

void GameRenderer::RenderDebugUILayer()
{

}

void GameRenderer::FinalRenderToBackBuffer()
{
    finalPassShader.UseShader();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, backBufferWidth, backBufferHeight);
    glDepthRange(0, 10);
    glClearColor(_RGB(255, 192, 203), 1.f);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Draw game frame
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gameLayer.colorTexId);
    screenSizeQuad.RenderMesh();

    // Draw UI frame
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, guiLayer.colorTexId);
    screenSizeQuad.RenderMesh();

    GLHasErrors();

    //LOCAL_PERSIST vec2 previousScreenSize = vec2();
    //if (previousScreenSize != vec2((float)backbufferWidth, (float)backbufferHeight))
    //{
    //    previousScreenSize = vec2((float)backbufferWidth, (float)backbufferHeight);
    //    float internal_ratio = (float)GAME_RESOLUTION_WIDTH / (float)GAME_RESOLUTION_HEIGHT;
    //    float screen_ratio = (float)backbufferWidth / (float)backbufferHeight;
    //    float finalOutputQuadVertices[16] = {
    //            -1.f, -1.f, 0.f, 0.f,
    //            1.f, -1.f, 1.f, 0.f,
    //            -1.f, 1.f, 0.f, 1.f,
    //            1.f, 1.f, 1.f, 1.f
    //    };
    //    if (screen_ratio > internal_ratio)
    //    {
    //        float w = backbufferHeight * internal_ratio;
    //        float f = (backbufferWidth - w) / backbufferWidth;
    //        finalOutputQuadVertices[0] = -1.f + f;
    //        finalOutputQuadVertices[4] = 1.f - f;
    //        finalOutputQuadVertices[8] = -1.f + f;
    //        finalOutputQuadVertices[12] = 1.f - f;
    //    }
    //    else
    //    {
    //        float h = backbufferWidth / internal_ratio;
    //        float f = (backbufferHeight - h) / backbufferHeight;
    //        finalOutputQuadVertices[1] = -1.f + f;
    //        finalOutputQuadVertices[5] = -1.f + f;
    //        finalOutputQuadVertices[9] = 1.f - f;
    //        finalOutputQuadVertices[13] = 1.f - f;
    //    }
    //    glBindVertexArray(finalQuadVAO);
    //    glBindBuffer(GL_ARRAY_BUFFER, finalQuadVBO);
    //    glBufferData(GL_ARRAY_BUFFER, 4 * 16, finalOutputQuadVertices, GL_DYNAMIC_DRAW);
    //    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
    //    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 6, refQuadIndices, GL_DYNAMIC_DRAW);
    //    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //    glBindVertexArray(0);
    //    printf("Screen size changed.\n");
    //}
}

void GameRenderer::CreateFrameBuffers()
{
    gameLayer.width = 800;
    gameLayer.height = 600;
    CreateBasicFrameBuffer(&gameLayer);
    guiLayer.width = 800;
    guiLayer.height = 600;
    CreateBasicFrameBuffer(&guiLayer);
    debugUILayer.width = 800;
    debugUILayer.height = 600;
    CreateBasicFrameBuffer(&debugUILayer);
}

void GameRenderer::UpdateFrameBuffersSize()
{
    //gameLayer.width = ;
    //gameLayer.height = ;
    UpdateBasicFrameBufferSize(&gameLayer);
    UpdateBasicFrameBufferSize(&guiLayer);
    UpdateBasicFrameBufferSize(&debugUILayer);
}

void GameRenderer::CreateBasicFrameBuffer(BasicFrameBuffer* buffer)
{
    buffer->FBO = 0;
    glGenFramebuffers(1, &buffer->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->FBO);

    glGenTextures(1, &buffer->colorTexId);
    glBindTexture(GL_TEXTURE_2D, buffer->colorTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, buffer->width, buffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenRenderbuffers(1, &buffer->depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer->depthRBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, buffer->colorTexId, 0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, buffer->width, buffer->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->depthRBO);

    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void GameRenderer::UpdateBasicFrameBufferSize(BasicFrameBuffer* buffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->FBO);
    glBindTexture(GL_TEXTURE_2D, buffer->colorTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, buffer->width, buffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer->depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, buffer->width, buffer->height);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        PrintLog.Error("Failed to change size of Internal FrameBuffer Object.");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GameRenderer::CreateMiscellaneous()
{
    u32 refQuadIndices[6] = {
            0, 1, 3,
            0, 3, 2
    };
    float refQuadVertices[16] = {
        //  x   y    u    v
        -1.f, -1.f, 0.f, 0.f,
        1.f, -1.f, 1.f, 0.f,
        -1.f, 1.f, 0.f, 1.f,
        1.f, 1.f, 1.f, 1.f
    };

    Mesh::MeshCreate(screenSizeQuad, refQuadVertices, refQuadIndices, 16, 6, 2, 2, 0, GL_STATIC_DRAW);
}
