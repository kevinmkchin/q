#pragma once

#define null_ui_id -1
typedef int ui_id;

#include <SDL.h>

#include <gmath.h>
#include "Common.h"

namespace KevGui
{
    enum class TextAlignment
    {
        Left,
        Center,
        Right
    };

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

    // Note(Kevin): I'm going to use the Do prefix for the primitive "building block" GUI elements
    // more complete GUI elements will not have this prefix.
    void DoPanel(UIRect rect, vec4 colorRGBA);

    void DoPanel(UIRect rect, int cornerWidth, u32 glTextureId = 0, float normalizedCornerSizeInUV = 0.3f);

    bool DoButton(ui_id id, UIRect rect, vec4 normalColor, vec4 hoveredColor, vec4 activeColor);

    void DoText(int x, int y, int size, TextAlignment alignment, const char* textFmt, ...);

    void DoTextUnformatted(const char* text, int x, int y, int size, TextAlignment alignment);

    void DoImage(UIRect rect, u32 glTextureId = 0);

    void DoIntegerField(ui_id id, UIRect rect, int* v);


    bool Temp_TextButton(UIRect rect, const char* label);

    void Temp_IncrementableIntegerField(const char* label, int* v);

    //void DoCheckbox(const char* label, );
    
    // void DoEditorPanel();
}

