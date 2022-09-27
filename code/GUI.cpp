#include "GUI.h"

#include <queue>

#include <gmath.h>
#include <vertext.h>
#include <stb_sprintf.h>

#include "Engine.h"
#include "GameRenderer.h"
#include "Shader.h"
#include "Resources.h"
#if INTERNAL_BUILD & SLOW_BUILD
#include "MessageLogger.h"
#endif

namespace KevGui
{
    static Shader __main_ui_shader;
    static const char* __main_ui_shader_vs =
        "#version 330 core\n"
        "layout (location = 0) in vec2 pos;\n"
        "layout (location = 1) in vec2 uv;\n"
        "out vec2 texUV;\n"
        "uniform mat4 matrixOrtho;\n"
        "void main() {\n"
        "   gl_Position = matrixOrtho * vec4(pos, 0.0, 1.0);\n"
        "   texUV = uv;\n"
        "}\n";
    static const char* __main_ui_shader_fs =
        "#version 330 core\n"
        "in vec2 texUV;\n"
        "out vec4 colour;\n"
        "uniform sampler2D textureSampler0;\n"
        "uniform bool useColour = false;\n"
        "uniform vec4 uiColour;\n"
        "void main() {\n"
        "   if (useColour) {\n"
        "       colour = uiColour;\n"
        "   } else {\n"
        "       colour = texture(textureSampler0, texUV);\n"
        "   }\n"
        "}\n";

    static Shader __text_shader;
    static const char* __text_shader_vs =
        "#version 330 core\n"
        "layout (location = 0) in vec2 pos;\n"
        "layout (location = 1) in vec2 uv;\n"
        "out vec2 texUV;\n"
        "uniform mat4 matrixModel;\n"
        "uniform mat4 matrixOrtho;\n"
        "void main() {\n"
        "   gl_Position = matrixOrtho * matrixModel * vec4(pos, 0.0, 1.0);\n"
        "   texUV = uv;\n"
        "}\n";
    static const char* __text_shader_fs =
        "#version 330 core\n"
        "in vec2 texUV;\n"
        "out vec4 colour;\n"
        "uniform sampler2D textureSampler0;\n"
        "uniform vec4 uiColour;\n"
        "void main() {\n"
        "   float textAlpha = texture(textureSampler0, texUV).x;\n"
        "   colour = vec4(uiColour.xyz, uiColour.w * textAlpha);\n"
        "}\n";

    static Mesh __ui_mesh;
    static Mesh __text_mesh;

    static Font __text_font;

    static char __reservedFormattedTextMem[16384];
    static const int __formattedTextLineSizeBytes = 128;
    static int __reservedFormattedTextMemIndexer = 0;


    struct UIDrawRequest
    {
        vec4 color;

        virtual void Draw() = 0;
    };

    struct RectDrawRequest : UIDrawRequest
    {
        UIRect rect;

        void Draw()
        {
            float left = (float)rect.x;
            float top = (float)rect.y;
            float bottom = (float)rect.y + rect.h;
            float right = (float)rect.x + rect.w;
            float vb[] = { left, top, 0.f, 1.f,
                           left, bottom, 0.f, 0.f,
                           right, bottom, 1.f, 0.f,
                           right, top, 1.f, 1.f, };
            u32 ib[] = { 0, 1, 3, 1, 2, 3 };

            __main_ui_shader.UseShader();
            __main_ui_shader.GLBind1i("useColour", true);
            __main_ui_shader.GLBind4f("uiColour", color.x, color.y, color.z, color.w);

            __ui_mesh.RebindBufferObjects(vb, ib, ARRAY_COUNT(vb), ARRAY_COUNT(ib), GL_DYNAMIC_DRAW);
            __ui_mesh.RenderMesh();
        }
    };

    struct CorneredRectDrawRequest : UIDrawRequest
    {
        UIRect rect;
        int cornerWidth = 10;

        GLuint textureId = 0;
        float normalizedCornerSizeInUV = 0.3f; // [0,1] with 0.5 being half way across texture

        void Draw()
        {
            float left = (float)rect.x;
            float top = (float)rect.y;
            float bottom = (float)rect.y + rect.h;
            float right = (float)rect.x + rect.w;
            float corner = (float)cornerWidth;
            float uv0 = normalizedCornerSizeInUV;
            float uv1 = 1.f - uv0;

            float vb[] = { left, top,              0.f, 1.f,
                           left, top + corner,     0.f, uv1,
                           left + corner, top,     uv0, 1.f,
                           left + corner, top + corner, uv0, uv1,

                           right - corner, top,    uv1, 1.f,
                           right - corner, top + corner, uv1, uv1,
                           right, top,             1.f, 1.f,
                           right, top + corner,    1.f, uv1, 

                           left, bottom - corner,  0.f, uv0, 
                           left, bottom,           0.f, 0.f,
                           left + corner, bottom - corner, uv0, uv0,
                           left + corner, bottom,  uv0, 0.f,

                           right - corner, bottom - corner, uv1, uv0,
                           right - corner, bottom, uv1, 0.f,
                           right, bottom - corner, 1.f, uv0,
                           right, bottom,          1.f, 0.f,
                        };
            u32 ib[] = { 0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7, 1, 8, 3, 3, 8, 10, 3, 10, 5, 
                5, 10, 12, 5, 12, 7, 7, 12, 14, 8, 9, 10, 10, 9, 11, 10, 11, 12, 12, 11, 13, 12, 13, 14, 14, 13, 15 };

            __main_ui_shader.UseShader();

            if (textureId != 0)
            {
                __main_ui_shader.GLBind1i("useColour", false);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textureId);
                __main_ui_shader.GLBind1i("textureSampler0", 0);
            }
            else
            {   
                __main_ui_shader.GLBind1i("useColour", true);
                __main_ui_shader.GLBind4f("uiColour", color.x, color.y, color.z, color.w);
            }

            __ui_mesh.RebindBufferObjects(vb, ib, ARRAY_COUNT(vb), ARRAY_COUNT(ib), GL_DYNAMIC_DRAW);
            __ui_mesh.RenderMesh();
        }
    };

    struct TextDrawRequest : UIDrawRequest
    {
        const char* text = "";
        int size = 8;
        int x = 0;
        int y = 0;

        void Draw()
        {
            vtxt_setflags(VTXT_CREATE_INDEX_BUFFER);
            vtxt_clear_buffer();
            vtxt_move_cursor(x, y);
            vtxt_append_line(text, __text_font.ptr, size);
            vtxt_vertex_buffer _txt = vtxt_grab_buffer();
            __text_mesh.RebindBufferObjects(_txt.vertex_buffer, _txt.index_buffer, _txt.vertices_array_count, _txt.indices_array_count, GL_DYNAMIC_DRAW);
            vtxt_clear_buffer();

            mat4 matrixModel = mat4();
            __text_shader.UseShader();
            __text_shader.GLBindMatrix4fv("matrixModel", 1, matrixModel.ptr());
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, __text_font.texture.textureId);
            __text_shader.GLBind1i("textureSampler0", 0);
            __text_shader.GLBind4f("uiColour", color.x, color.y, color.z, color.w);

            __text_mesh.RenderMesh();
        }
    };

    static std::vector<RectDrawRequest> rectDrawRequestsBuffer;
    static std::vector<CorneredRectDrawRequest> corneredRectDrawRequestsBuffer;
    static std::vector<TextDrawRequest> textDrawRequestsBuffer;
    static std::queue<UIDrawRequest*> drawQueue;


    static ui_id freshIdCounter = 0;
    static ui_id hoveredUI = null_ui_id;
    static ui_id activeUI = null_ui_id;
    static bool anyElementHoveredThisFrame = false;
    static bool IsActive(ui_id id)
    {
        return activeUI == id;
    }
    static bool IsHovered(ui_id id)
    {
        return hoveredUI == id;
    }
    static void SetActive(ui_id id)
    {
        activeUI = id;
    }
    static void SetHovered(ui_id id)
    {
        anyElementHoveredThisFrame = true;
        hoveredUI = id;
    }


    static int mousePosX = 0;
    static int mousePosY = 0;
    static bool mouseDown = false;
    static bool mouseUp = false;
    static bool MouseWentUp()
    {
        return mouseUp;
    }
    static bool MouseWentDown()
    {
        return mouseDown;
    }
    static bool MouseInside(const UIRect& rect)
    {
        int left = rect.x;
        int top = rect.y;
        int right = left + rect.w;
        int bottom = top + rect.h;
        if (left <= mousePosX && mousePosX <= right
            && top <= mousePosY && mousePosY <= bottom)
        {
            return true;
        }
        else
        {
            return false;
        }
    }


    bool DoButton(ui_id id, UIRect rect)
    {
        bool result = false;

        if(IsActive(id))
        {
            if(MouseWentUp())
            {
                if(IsHovered(id))
                {
                    result = true;
                }
                SetActive(null_ui_id);
            }
        }
        else if(IsHovered(id))
        {
            if(MouseWentDown())
            {
                SetActive(id);
            }
        }

        if(MouseInside(rect))
        {
            SetHovered(id);
        }

        RectDrawRequest drawRequest;
        drawRequest.rect = rect;
        drawRequest.color = IsHovered(id) ? vec4(0.5f,0.5f,0.5f,1.f) : vec4(1.f,1.f,1.f,1.f);
        if (IsActive(id)) drawRequest.color = vec4(0.2f,0.2f,0.2f,1.f);

        rectDrawRequestsBuffer.push_back(drawRequest);
        drawQueue.push(&rectDrawRequestsBuffer.back());

        return result;
    }


    void DoPanel(UIRect rect, vec4 colorRGBA)
    {
        RectDrawRequest drawRequest;
        drawRequest.rect = rect;
        drawRequest.color = colorRGBA;

        rectDrawRequestsBuffer.push_back(drawRequest);
        drawQueue.push(&rectDrawRequestsBuffer.back());
    }

    void DoPanel(UIRect rect, int cornerWidth, u32 glTextureId, float normalizedCornerSizeInUV)
    {
        CorneredRectDrawRequest drawRequest;
        drawRequest.rect = rect;
        drawRequest.color = vec4(0.157f, 0.172f, 0.204f, 1.f);
        drawRequest.textureId = glTextureId;
        drawRequest.cornerWidth = cornerWidth;
        drawRequest.normalizedCornerSizeInUV = normalizedCornerSizeInUV;

        corneredRectDrawRequestsBuffer.push_back(drawRequest);
        drawQueue.push(&corneredRectDrawRequestsBuffer.back());
    }

    void DoText(int x, int y, int size, const char* textFmt, ...)
    {
#if INTERNAL_BUILD & SLOW_BUILD
        if (__reservedFormattedTextMemIndexer >= ARRAY_COUNT(__reservedFormattedTextMem) / __formattedTextLineSizeBytes)
        {
            PrintLog.WarningFmt("Attempting to format text in GUI.cpp but not enough reserved memory to store formatted text.");
        }
#endif

        va_list argptr;
        char* formattedTextBuffer = __reservedFormattedTextMem + (__reservedFormattedTextMemIndexer * __formattedTextLineSizeBytes);
        va_start(argptr, textFmt);
        stbsp_vsprintf(formattedTextBuffer, textFmt, argptr);
        va_end(argptr);
        ++__reservedFormattedTextMemIndexer;

        DoTextUnformatted(formattedTextBuffer, x, y, size);
    }

    void DoTextUnformatted(const char* text, int x, int y, int size)
    {
        TextDrawRequest drawRequest;
        drawRequest.text = text;
        drawRequest.size = size;
        drawRequest.x = x;
        drawRequest.y = y;
        drawRequest.color = vec4(1.f, 1.f, 1.f, 1.f);

        textDrawRequestsBuffer.push_back(drawRequest);
        drawQueue.push(&textDrawRequestsBuffer.back());
    }


    ui_id FreshID()
    {
        return freshIdCounter++;
    }

    void Init()
    {
        hoveredUI = null_ui_id;
        activeUI = null_ui_id;
        Shader::GLCreateShaderProgram(__main_ui_shader, __main_ui_shader_vs, __main_ui_shader_fs);
        Shader::GLCreateShaderProgram(__text_shader, __text_shader_vs, __text_shader_fs);
        Mesh::MeshCreate(__ui_mesh, nullptr, nullptr, 0, 0, 2, 2, 0, GL_DYNAMIC_DRAW);
        Mesh::MeshCreate(__text_mesh, nullptr, nullptr, 0, 0, 2, 2, 0, GL_DYNAMIC_DRAW);

        __text_font = Font::FontCreateFromFile(font_path("c64.ttf"), 16, true);
    }

    void NewFrame()
    {
        if (!anyElementHoveredThisFrame)
        {
            hoveredUI = null_ui_id;
        }
        anyElementHoveredThisFrame = false;

        rectDrawRequestsBuffer.clear();
        corneredRectDrawRequestsBuffer.clear();

        freshIdCounter = 0;

        __reservedFormattedTextMemIndexer = 0;
    }

    void SDLProcessEvent(const SDL_Event* evt)
    {
        SDL_Event event = *evt;
        switch (event.type)
        {
        case SDL_MOUSEMOTION: {
            mousePosX = event.motion.x;
            mousePosY = event.motion.y;
        }break;
        case SDL_MOUSEBUTTONDOWN:
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouseDown = true;
                mouseUp = false;
            }
        }break;
        case SDL_MOUSEBUTTONUP:
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                mouseUp = true;
                mouseDown = false;
            }
        }break;
        }
    }

    void Draw()
    {
        i32 kevGuiScreenWidth = Engine.gfx->guiLayer.width;
        i32 kevGuiScreenHeight = Engine.gfx->guiLayer.height;
        mat4 matrixOrtho = ProjectionMatrixOrthographic2D(0.f, (float)kevGuiScreenWidth, (float)kevGuiScreenHeight, 0.f);

        __main_ui_shader.UseShader();
        __main_ui_shader.GLBindMatrix4fv("matrixOrtho", 1, matrixOrtho.ptr());

        __text_shader.UseShader();
        __text_shader.GLBindMatrix4fv("matrixOrtho", 1, matrixOrtho.ptr());

        while(!drawQueue.empty())
        {
            UIDrawRequest* drawCall = drawQueue.front();
            drawQueue.pop();
            drawCall->Draw();
        }
    }

}