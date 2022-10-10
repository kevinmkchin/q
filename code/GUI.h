#pragma once

#define null_ui_id -1
typedef int ui_id;

#include <SDL.h>

#include <gmath.h>
#include "Common.h"
#include "Resources.h"

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

    struct UIStyle
    {
        vec4 buttonNormalColor = vec4(0.18f, 0.18f, 0.18f, 1.f);
        vec4 buttonHoveredColor = vec4(0.08f, 0.08f, 0.08f, 1.f);
        vec4 buttonActiveColor = vec4(0.06f, 0.06f, 0.06f, 1.f);
        Font textFont;
        vec4 textColor = vec4(1.f, 1.f, 1.f, 1.f);

        vec4 editorWindowBackgroundColor = vec4(0.f, 1.f, 0.f, 1.f);
        int editorTextSize = 16;

    };

    struct UIWindowGuide
    {
        ui_id windowId = null_ui_id;
        UIRect initialWindowRect;
        int xOffsetFromInitialWindow;
        int yOffsetFromInitialWindow;
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


    void PushUIStyle(UIStyle style);
    void PopUIStyle();
    UIStyle GetActiveUIStyleCopy();
    UIStyle& GetActiveUIStyleReference();

    bool LabelledButton(UIRect rect, const char* label, int textSize, TextAlignment textAlignment);

    void IncrementableIntegerField(const char* label, int* v);

    //void DoCheckbox(const char* label, );
    
    // void DoEditorPanel();

    void EditorBeginWindow(UIRect windowRect);

    void EditorEndWindow();

    // void EditorText(const char* textFmt, ...);

    bool EditorLabelledButton(const char* label);
}

