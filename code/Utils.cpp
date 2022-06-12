#include "Utils.h"

#include <assert.h>
#include <glew.h>

#include "Common.h"


bool GLHasErrors()
{
    GLenum error = glGetError();

    if (error == GL_NO_ERROR) return false;

    while (error != GL_NO_ERROR)
    {
        const char* error_str = "";
        switch (error)
        {
        case GL_INVALID_OPERATION:
            error_str = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error_str = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error_str = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error_str = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error_str = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }

        PrintLog.ErrorFmt("OpenGL: %s", error_str);
        error = glGetError();
        assert(false);
    }

    return true;
}

void PickThreeRandomInts(int* x, int* y, int* z, int cap)
{
    assert(cap >= 3);
    *x = rand()%cap;
    do {
        *y = rand()%cap;
    } while(*y == *x);
    do {
        *z = rand()%cap;
    } while(*z == *x || *z == *y);
}

int RandomInt(int min, int max)
{
    int retval = min + (rand() % static_cast<int>(max - min + 1));
    return retval;
}
