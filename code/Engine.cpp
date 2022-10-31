/**  TODO

- Change it so that even if selected point is not aligned to grid, the next desired move point MUST BE aligned to grid
    just snap the values of next desired move point to be aligned to grid.

    /// TODO(Kevin)
    ///
    /// -selected brush shows up as translucent (only translucent to show its own edges, not brushes behind it)
    /// -brush edges show up in red
    /// -brush surfaces are highlighted in red
    /// -yellow filled circular handles show up; occluded circles are darker
    /// -hover mouse near circular handle to show bigger red unfilled circle around yellow circle; hover one at a time (follow z order)
    /// -yellow circular handle turns red upon click and release
    /// -upon click, user can drag mouse to move circle/vertex
    /// -with vertex tool, clicking and holding down left click on circle shows translucent yellow lines along the axes
    ///
    /// -update intersections function
    /// -face tool and edge tool is just vertex tool w diff num of vertices selected
    ///
    /// -the things that move show up in yellow, so vertices if vertex mode, edges if edge mode, faces if face mode
    ///
    /// Do something about vertex coordinates becoming hella imprecise

    // place points and extrude tool: extrude in direction of normal (cross product) thus extrude only available when placed points are 2D.
    //                                if points are in a 3D cloud, then do a bunch of cross products to get a bunch of planes, clean up unused or duplicate planes,
    //                                then update intersections,faces,vertices,etc.
    // clip tool: add new plane (the slice), update intersection & vertices etc information, then remove all planes that no longer have a face/vertices
    // vertex tool: move vertices and add planes accordingly (adding planes might just be splitting existing planes into two), then update intersection i think
    //              may need to merge vertices or planes together into one if they become same i.e. if two planes become same, prob need to combine into one plane
    // edge tool: vertex tool but with multiple vertices selected
    // face tool: vertex tool but with multiple vertices selected
    // if multiple brushes are selected, and we want to move an vertex/edge, same vertices between brushes should be treated as one

GUI
- Capture mouse click -> save visible window areas of previous frame
- editor gui elements must belong to an editor window
    - when windows are overlapping, the gui elements belonging to the occluded
      windows may not receive mouse click (if mouse click pos is within topmost window bb)
- Button that repeatedly returns true if held

- Checkbox
- Dropdown
- List
- Selectable

- make pretty
- anchors
    - push pop anchor point, at which point all ements in between the push pop use that anchor

GAME & EDITOR
- WorldLevelGeometryCollider that has all its points in WCS
- Asset management? do i need to?

*/

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
#include "HandleUtility.h"
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
        "Quake+Bioshock Clone",
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
    SDL_GL_SetSwapInterval(0);

    return true;
}

void EngineClass::SDLProcessEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (console_is_hidden())
        {
            Input.ProcessAllSDLInputEvents(event);
            KevGui::SDLProcessEvent(&event);
        }

        // Lower level engine related input
        switch (event.type) 
        {
            case SDL_KEYDOWN: {
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
    
//    GameRuntime gameRuntime;
//    game = &gameRuntime;
//    game->LoadAllContent();
//    game->StartGame();
    
    EditorRuntime editorRuntime;
    editor = &editorRuntime;
    editor->StartEditor();
    
    console_initialize();

    Handles::Init();
    KevGui::Init();

    RegisterConsoleCommandsAndVariables();

    while (!programShouldShutdown) 
    {
        if (Time.UpdateDeltaTime() > 0.1f) { continue; } // if delta time is too large, will cause glitches

        KevGui::NewFrame();

        SDLProcessEvents();

        console_update(Time.unscaledDeltaTime);

        editorRuntime.UpdateEditor(); //game->Update();
        DrawProfilerGUI();
        gfx->Render();

        SDL_GL_SwapWindow(window);

        Input.ResetInputStatesAtEndOfFrame();
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

void SetSDLFullscreen(int fullscreen)
{
    switch (fullscreen)
    {
        case 0: {SDL_SetWindowFullscreen(Engine.window, 0); }break;
        case 1: {SDL_SetWindowFullscreen(Engine.window, SDL_WINDOW_FULLSCREEN_DESKTOP); }break;
        case 2: {SDL_SetWindowFullscreen(Engine.window, SDL_WINDOW_FULLSCREEN); }break;
        default: {
            PrintLog.Message("Invalid fullscreen mode; 0 = windowed, 1 = fullscreen, 2 = fullscreen-windowed");
        }break;
    }
}

void SetSDLWindowSize(int w, int h)
{
    SDL_SetWindowSize(Engine.window, w, h);
}

void SetSDLVSync(int vsync)
{
    switch (vsync) { // 0 = immediate (no vsync), 1 = vsync, 2 = adaptive vsync
        case 0: {SDL_GL_SetSwapInterval(0); }break;
        case 1: {SDL_GL_SetSwapInterval(1); }break;
        case 2: {if (SDL_GL_SetSwapInterval(-1) == -1) SDL_GL_SetSwapInterval(1); }break;
        default: {
            console_print("Invalid vsync option; 0 = immediate, 1 = vsync, 2 = adaptive vsync\n");
        }break;
    }
}

void EngineClass::RegisterConsoleCommandsAndVariables()
{
    get_console().bind_cmd("fullscreen", SetSDLFullscreen);
    get_console().bind_cmd("windowsize", SetSDLWindowSize);
    get_console().bind_cmd("vsync", SetSDLVSync);
    get_console().bind_cvar("profiler", &profilerLevel);
}

void EngineClass::DrawProfilerGUI()
{
    if (profilerLevel >= 1)
    {
        static char cbuf[64];
        static float lastTime = -1.f;
        if (Time.time - lastTime > (profilerLevel == 1 ? 1.0f : 0.0f))
        {
            lastTime = Time.time;
            stbsp_sprintf(cbuf, "LAST FRAME TIME: %.2fms   FPS: %dhz", 1000.f * Time.unscaledDeltaTime, (i16)(1.f / Time.unscaledDeltaTime));
        }
        float cbufw;
        float cbufh;
        vtxt_get_text_bounding_box_info(&cbufw, &cbufh, cbuf, KevGui::GetActiveUIStyleReference().textFont.ptr, 12);
        KevGui::DoPanel(KevGui::UIRect(0, 0, (int)cbufw + 2, 18), KevGui::GetActiveUIStyleReference().editorWindowBackgroundColor);
        KevGui::DoTextUnformatted(4, 14, 12, KevGui::TextAlignment::Left, cbuf);
    }
    if (profilerLevel >= 2)
    {

    }
}

int main(int argc, char* argv[])
{
    return Engine.Run();
}
