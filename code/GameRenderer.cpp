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


    CreateFrameBuffers();
    UpdateFrameBuffersSize();


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
//    // Setting shaders
//    // get the wind texture, sprite mesh, and program
//    glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::FINAL_PASS]);
//    // Clearing backbuffer
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    glViewport(0, 0, backbufferWidth, backbufferHeight);
//    glDepthRange(0, 10);
//    glClearColor(0.f, 0.f, 0.f, 1.f);
//    glClearDepth(1.f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    // Enabling alpha channel for textures
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glDisable(GL_DEPTH_TEST);
//
//    LOCAL_PERSIST u32 finalQuadVAO;
//    LOCAL_PERSIST u32 finalQuadVBO;
//    LOCAL_PERSIST u32 finalQuadIBO;
//
//    u32 refQuadIndices[6] = {
//            0, 1, 3,
//            0, 3, 2
//    };
//    float refQuadVertices[16] = {
//        //  x   y    u    v
//        -1.f, -1.f, 0.f, 0.f,
//        1.f, -1.f, 1.f, 0.f,
//        -1.f, 1.f, 0.f, 1.f,
//        1.f, 1.f, 1.f, 1.f
//    };
//
//    if (!finalQuadVAO)
//    {
//        glGenVertexArrays(1, &finalQuadVAO);
//        glBindVertexArray(finalQuadVAO);
//        glGenBuffers(1, &finalQuadVBO);
//        glBindBuffer(GL_ARRAY_BUFFER, finalQuadVBO);
//        glBufferData(GL_ARRAY_BUFFER, 4 /*bytes cuz float*/ * 16, refQuadVertices, GL_DYNAMIC_DRAW);
//        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, nullptr);
//        glEnableVertexAttribArray(0);
//        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
//        glEnableVertexAttribArray(1);
//
//        glGenBuffers(1, &finalQuadIBO);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 /*bytes cuz uint32*/ * 6, refQuadIndices, GL_DYNAMIC_DRAW);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindVertexArray(0); // Unbind the VAO;
//    }
//
//    LOCAL_PERSIST vec2 previousScreenSize = vec2();
//    if (previousScreenSize != vec2((float)backbufferWidth, (float)backbufferHeight))
//    {
//        previousScreenSize = vec2((float)backbufferWidth, (float)backbufferHeight);
//        float internal_ratio = (float)GAME_RESOLUTION_WIDTH / (float)GAME_RESOLUTION_HEIGHT;
//        float screen_ratio = (float)backbufferWidth / (float)backbufferHeight;
//        float finalOutputQuadVertices[16] = {
//                -1.f, -1.f, 0.f, 0.f,
//                1.f, -1.f, 1.f, 0.f,
//                -1.f, 1.f, 0.f, 1.f,
//                1.f, 1.f, 1.f, 1.f
//        };
//        if (screen_ratio > internal_ratio)
//        {
//            float w = backbufferHeight * internal_ratio;
//            float f = (backbufferWidth - w) / backbufferWidth;
//            finalOutputQuadVertices[0] = -1.f + f;
//            finalOutputQuadVertices[4] = 1.f - f;
//            finalOutputQuadVertices[8] = -1.f + f;
//            finalOutputQuadVertices[12] = 1.f - f;
//        }
//        else
//        {
//            float h = backbufferWidth / internal_ratio;
//            float f = (backbufferHeight - h) / backbufferHeight;
//            finalOutputQuadVertices[1] = -1.f + f;
//            finalOutputQuadVertices[5] = -1.f + f;
//            finalOutputQuadVertices[9] = 1.f - f;
//            finalOutputQuadVertices[13] = 1.f - f;
//        }
//        glBindVertexArray(finalQuadVAO);
//        glBindBuffer(GL_ARRAY_BUFFER, finalQuadVBO);
//        glBufferData(GL_ARRAY_BUFFER, 4 * 16, finalOutputQuadVertices, GL_DYNAMIC_DRAW);
//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * 6, refQuadIndices, GL_DYNAMIC_DRAW);
//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindVertexArray(0);
//        printf("Screen size changed.\n");
//    }
//
//    GLint currProgram;
//    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
//    GLuint darkenFactor_loc = glGetUniformLocation(currProgram, "darkenFactor");
//
//    // Bind game frame texture in Texture Unit 0
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, offScreenRenderBufferColor);
//    bool darkenGameFrame = false;
//    darkenGameFrame ? glUniform1f(darkenFactor_loc, 0.5f) : glUniform1f(darkenFactor_loc, 0.0f);
//
//    // Draw game frame
//    glBindVertexArray(finalQuadVAO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
//    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    // Bind UI frame texture in Texture Unit 0
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, offScreenUiBufferColor);
//    glUniform1f(darkenFactor_loc, 0.0f);
//
//    // Draw UI frame
//    glBindVertexArray(finalQuadVAO);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, finalQuadIBO);
//    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    glHasErrors();
}

void GameRenderer::CreateFrameBuffers()
{

}

void GameRenderer::UpdateFrameBuffersSize()
{

}
