#pragma once

#define null_ui_id -1
typedef int ui_id;

#include <SDL.h>

#include <gmath.h>
#include "Common.h"

namespace KevGui
{
    struct UIRect
    {
        UIRect() : x(0), y(0), w(0), h(0) {};
        UIRect(int _x, int _y, int _w, int _h) : x(_x), y(_y), w(_w), h(_h) {};
        int x;
        int y;
        int w;
        int h;
    };

    ui_id FreshID();

    void Init();

    void NewFrame();

    void SDLProcessEvent(const SDL_Event* evt);

    void Draw();


    void DoPanel(UIRect rect, vec4 colorRGBA);

    void DoPanel(UIRect rect, int cornerWidth, u32 glTextureId = 0, float normalizedCornerSizeInUV = 0.3f);

    bool DoButton(ui_id id, UIRect rect);

    void DoText(int x, int y, int size, const char* textFmt, ...);

    void DoTextUnformatted(const char* text, int x, int y, int size/*, font*/);


    
    // void DoEditorPanel();
}

