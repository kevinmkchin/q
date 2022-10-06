#ifndef GAME_KEVINOPENGL_H
#define GAME_KEVINOPENGL_H

#if _WIN32
#include <glew.h>
#define KEVIN_USING_GLEW
#elif __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif

#endif //GAME_KEVINOPENGL_H
