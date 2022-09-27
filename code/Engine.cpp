#include <iostream>

#include <SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#define VERTEXT_IMPLEMENTATION
#include <vertext.h>

#include "Common.h"
#include "Engine.h"
#include "Input.h"
#include "GameRenderer.h"
#include "GameRuntime.h"
#include "Resources.h"
#include "Console.h"
#include "GUI.h"
#include "EditorRuntime.h"

EngineClass Engine;

bool EngineClass::SDLInitialize()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        PrintLog.Error("SDL failed to initialize.");
        return false;
    }

    // OpenGL Context Attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Setup SDL Window
    if ((window = SDL_CreateWindow(
        "Quake Clone",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    )) == nullptr)
    {
        PrintLog.Error("SDL window failed to create.");
        return false;
    }
    SDL_SetWindowResizable(window, SDL_TRUE);
    PrintLog.Message("SDL window created.");

    if ((openglContext = SDL_GL_CreateContext(window)) == nullptr)
    {
        PrintLog.Error("Failed to create OpenGL context with SDL.");
        return false;
    }
    PrintLog.Message("OpenGL context created.");

    //// Loading music and sounds with SDL
    //if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    //    fprintf(stderr, "Failed to initialize SDL Audio");
    //    return false;
    //}
    //if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
    //    fprintf(stderr, "Failed to open audio device");
    //    return false;
    //}

    // Window Icon
    int req_format = STBI_rgb_alpha;
    void* iconMemory = nullptr;
    int iconWidth = 0;
    int iconHeight = 0;
    int iconBitDepth = 0;
    iconMemory = stbi_load(data_path("icon.bmp").c_str(), (int*)&iconWidth, (int*)&iconHeight, (int*)&iconBitDepth, req_format);
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (req_format == STBI_rgb) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
#else // little endian, like x86
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = (req_format == STBI_rgb) ? 0 : 0xff000000;
#endif
    int depth, pitch;
    if (req_format == STBI_rgb) {
        depth = 24;
        pitch = 3 * iconWidth; // 3 bytes per pixel * pixels per row
    }
    else { // STBI_rgb_alpha (RGBA)
        depth = 32;
        pitch = 4 * iconWidth;
    }
    SDL_Surface* windowIcon = SDL_CreateRGBSurfaceFrom(iconMemory, iconWidth, iconHeight, depth, pitch, rmask, gmask, bmask, amask);
    SDL_SetWindowIcon(window, windowIcon);

    return true;
}

void EngineClass::SDLProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        KevGui::SDLProcessEvent(&event);

        switch (event.type) 
        {
            case SDL_KEYDOWN: {
                Input::ProcessSDLKeyDownEvent(event.key);

                if (event.key.keysym.sym == SDLK_BACKQUOTE)
                {
                    console_toggle();
                    break;
                }

                if (console_is_shown())
                {
                    console_keydown(event.key);
                    break;
                }

                if (event.key.keysym.sym == SDLK_F1)
                {
                    SDL_SetWindowFullscreen(window, 0);
                }
                if (event.key.keysym.sym == SDLK_F2)
                {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                if(event.key.keysym.sym == SDLK_F3)
                {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                }
            }break;

            case SDL_KEYUP: {
                Input::ProcessSDLKeyUpEvent(event.key);
            }break;

            case SDL_CONTROLLERBUTTONDOWN: {
                Input::ProcessSDLControllerButtonDownEvent(event.cbutton);
            }break;
            case SDL_CONTROLLERBUTTONUP: {
                Input::ProcessSDLControllerButtonUpEvent(event.cbutton);
            }break;
            case SDL_CONTROLLERAXISMOTION: {
                Input::ProcessSDLControllerAxisEvent(event.caxis);
            }break;
            case SDL_CONTROLLERDEVICEADDED: {
                Input::SDLControllerConnected(event.cdevice.which);
            }break;
            case SDL_CONTROLLERDEVICEREMOVED: {
                Input::SDLControllerRemoved(event.cdevice.which);
            }break;

            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        gfx->UpdateBackBufferSize();
                    }break;
                }
            }break;

            case SDL_QUIT: {
                programShouldShutdown = true;
            }break;
        }
    }
}

int EngineClass::Run()
{
    // Initialize SDL window and OpenGL context
    if (!SDLInitialize()) return EXIT_FAILURE;

    GameRenderer renderer;
    gfx = &renderer;
    if (!gfx->Init()) return EXIT_FAILURE;

    LoadPerpetualResources();

    GameRuntime gameRuntime;
    game = &gameRuntime;

    EditorRuntime editorRuntime;

    console_initialize();

    game->StartGame();

    KevGui::Init();

    while (!programShouldShutdown) 
    {
        if (Time.UpdateDeltaTime() > 0.1f) { continue; } // if delta time is too large, will cause glitches

        SDLProcessEvents();

        console_update(Time.unscaledDeltaTime);

        KevGui::NewFrame();

        editorRuntime.UpdateEditor(); //game->Update();
        gfx->Render();

        SDL_GL_SwapWindow(window);

        Input::ResetControllerStates();
        Input::ResetKeyboardStates();
    }

    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(openglContext);
    SDL_Quit();
    return EXIT_SUCCESS;
}

void EngineClass::LoadPerpetualResources()
{
    // don't need to unload. these are resources alive for the entire program. when Windows program closes, resources are automatically cleaned up!

    Resources.fonts[FONT_SourceCodePro] = Font::FontCreateFromFile(font_path("SourceCodePro.ttf"), 24, false);
    Resources.fonts[FONT_Iosevka] = Font::FontCreateFromFile(font_path("iosevka-term-regular.ttf"), 24, false);

    Texture::TextureCreateFromFile(Resources.defaultMissingTexture, primitives_path("missing_texture.png").c_str());
}

int main(int argc, char* argv[])
{
    return Engine.Run();
}
