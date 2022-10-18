#include "GameRenderer.h"

#include <kevinopengl.h>
#include <SDL.h>
#include <gmath.h>

#include "Common.h"
#include "Utils.h"
#include "Engine.h"
#include "Camera.h"
#include "Console.h"
#include "GameRuntime.h"
#include "EditorRuntime.h"
#include "GUI.h"
#include "LevelGeometry.h"

static const char* __finalpass_shader_vs = 
    "#version 330\n"
    "layout(location = 0) in vec2 pos;\n"
    "layout(location = 1) in vec2 uv;\n"
    "out vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(pos, 0, 1.0);\n"
    "    texcoord = uv;\n"
    "}\n";

static const char* __finalpass_shader_fs =
    "#version 330\n"
    "uniform sampler2D screen_texture;\n"
    "in vec2 texcoord;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "    vec4 in_color = texture(screen_texture, texcoord);\n"
    "    if(in_color.w < 0.001)\n"
    "    {\n"
    "        discard;\n"
    "    }\n"
    "    color = in_color;\n"
    "}\n";

bool GameRenderer::Init()
{
#ifdef KEVIN_USING_GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        PrintLog.Error("GLEW failed to initialize.");
        return false;
    }
    PrintLog.Message("GLEW initialized.");
#endif

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // alpha blending func: a * (rgb) + (1 - a) * (rgb) = final color output
    glBlendEquation(GL_FUNC_ADD);

    CreateFrameBuffers();
    UpdateBackBufferSize();

    Shader::GLCreateShaderProgram(finalPassShader, __finalpass_shader_vs, __finalpass_shader_fs);
    Shader::GLLoadShaderProgramFromFile(gameSceneShader, shader_path("scene.vert").c_str(), shader_path("scene.frag").c_str());

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
    glEnable(GL_CULL_FACE);

    if (activeGameCamera == nullptr) return;

    gameSceneShader.UseShader();

    gameSceneShader.GLBindMatrix4fv("viewMatrix", 1, activeGameCamera->viewMatrix.ptr());
    gameSceneShader.GLBindMatrix4fv("projMatrix", 1, activeGameCamera->perspectiveMatrix.ptr());

    //GameState* gameState = &(Engine.game->state);
    for (int i = 0; i < 1; ++i)
    {
        for (int j = 0; j < 1; ++j)
        {
            mat4 mm = mat4();
            mm *= TranslationMatrix(0.f, -256 * (j - 0), -256.f * (i - 0));
            gameSceneShader.GLBindMatrix4fv("modelMatrix", 1, mm.ptr());
            for (auto fb : Engine.editor->brush_0->facesForDrawing)
            {
                LevelGeometry::RenderFaceBatch(fb);
            }
        }
    }
    
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
