#include "GameRenderer.h"

#include <glew.h>

#include "Common.h"

bool GameRenderer::Init()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        PrintLog.Error("GLEW failed to initialize.\n");
        return false;
    }
    PrintLog.Message("GLEW initialized.\n");

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // alpha blending func: a * (rgb) + (1 - a) * (rgb) = final color output
    glBlendEquation(GL_FUNC_ADD);

    return true;
}

void GameRenderer::Render()
{
    glViewport(0, 0, 800, 600);
    glClearColor(_RGB(46, 88, 120), 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}
