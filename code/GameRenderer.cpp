#include "GameRenderer.h"

#include <glew.h>
#include <SDL.h>
#include <gmath.h>

#include "Common.h"
#include "Utils.h"
#include "Engine.h"
#include "Camera.h"
#include "Console.h"
#include "GameRuntime.h"
#include "GUI.h"

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

    CreateFrameBuffers();
    UpdateBackBufferSize();

    Shader::GLLoadShaderProgramFromFile(finalPassShader, shader_path("finalpass.vert").c_str(), shader_path("finalpass.frag").c_str());
    Shader::GLLoadShaderProgramFromFile(gameSceneShader, shader_path("scene.vert").c_str(), shader_path("scene.frag").c_str());
    Shader::GLLoadShaderProgramFromFile(consoleUIShader, shader_path("console_ui.vert").c_str(), shader_path("console_ui.frag").c_str());
    Shader::GLLoadShaderProgramFromFile(consoleTextUIShader, shader_path("console_text_ui.vert").c_str(), shader_path("console_text_ui.frag").c_str());

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
    glBindFramebuffer(GL_FRAMEBUFFER, gameLayer.FBO);
    glViewport(0, 0, gameLayer.width, gameLayer.height);
    glClearColor(_RGB(46, 88, 120), 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    GameState* gameState = &(Engine.game->state);

    LOCAL_PERSIST GameCamera camera;
    camera.position = vec3(-4.f,0.f,0.f);
    camera.CameraUpdate();
    camera.CameraCalculatePerspectiveMatrix();
    
    gameSceneShader.UseShader();

    gameSceneShader.GLBindMatrix4fv("viewMatrix", 1, camera.viewMatrix.ptr());
    gameSceneShader.GLBindMatrix4fv("projMatrix", 1, camera.perspectiveMatrix.ptr());

    //auto rotatingCube = std::static_pointer_cast<RotatingCube>(gameState->entities[gameState->rotatingCube]);
    //rotatingCube->graphics.Render(gameSceneShader);

    glUseProgram(0);
}

void GameRenderer::RenderGUILayer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, guiLayer.FBO);
    glViewport(0, 0, guiLayer.width, guiLayer.height);
    glDepthRange(0.00001f, 10.f);
    glClearColor(0.674f, 0.847f, 1.0f, 0.0f);
    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);


    KevGui::Draw();
}

void GameRenderer::RenderDebugUILayer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, debugUILayer.FBO);
    glViewport(0, 0, debugUILayer.width, debugUILayer.height);
    glDepthRange(0.00001f, 10.f);
    glClearColor(0.674f, 0.847f, 1.0f, 0.0f);
    glClearDepth(10.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // CONSOLE RENDERING
    glDisable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    console_render();
    glEnable(GL_DEPTH_TEST);
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

    // Draw GUI frame
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, guiLayer.colorTexId);
    screenSizeQuad.RenderMesh();

    // Draw Debug UI frame
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, debugUILayer.colorTexId);
    screenSizeQuad.RenderMesh();

    GLHasErrors();
}

void GameRenderer::UpdateBackBufferSize()
{
    SDL_GL_GetDrawableSize(Engine.window, &backBufferWidth, &backBufferHeight);
    UpdateFrameBuffersSize();
}

void GameRenderer::GetBackBufferSize(i32* widthOutput, i32* heightOutput)
{
    *widthOutput = backBufferWidth;
    *heightOutput = backBufferHeight;
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
    UpdateBasicFrameBufferSize(&gameLayer, backBufferWidth, backBufferHeight);
    UpdateBasicFrameBufferSize(&guiLayer, backBufferWidth, backBufferHeight); // TODO(Kevin): gui layer should use different resolution to game
    UpdateBasicFrameBufferSize(&debugUILayer, backBufferWidth, backBufferHeight);
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

void GameRenderer::UpdateBasicFrameBufferSize(BasicFrameBuffer* buffer, i32 newWidth, i32 newHeight)
{
    buffer->width = newWidth;
    buffer->height = newHeight;
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
