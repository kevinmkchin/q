#include "HandleUtility.h"

#include <kevinopengl.h>

#include "Common.h"
#include "Utils.h"
#include "Shader.h"
#include "Engine.h"
#include "GameRenderer.h"
#include "Input.h"

namespace Handles
{
    static u32 __handleVisualVaoId = 0;
    static u32 __handleVisualVboId = 0;

    static Shader __handle_shader;
    static const char* __handle_shader_vs =
        "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "uniform mat4 projectionMatrix;\n"
        "uniform mat4 viewMatrix;\n"
        "void main() {\n"
        "    gl_Position = projectionMatrix * viewMatrix * vec4(pos, 1.0);\n"
        "}\n";
    static const char* __handle_shader_fs =
        "#version 330 core\n"
        "out vec4 colour;\n"
        "uniform vec4 handleColour;\n"
        "void main() {\n"
        "    colour = handleColour;\n"
        "}\n";

    static NiceArray<vec4, 128> __colors;
    static NiceArray<u16, 128> __verticesCounts; // vertex buffer sizes in number of floats
    static NiceArray<float, 256 * 128> __vb;

    void Init()
    {
        glGenVertexArrays(1, &__handleVisualVaoId);
        glBindVertexArray(__handleVisualVaoId);
        glGenBuffers(1, &__handleVisualVboId);
        glBindBuffer(GL_ARRAY_BUFFER, __handleVisualVboId);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        Shader::GLCreateShaderProgram(__handle_shader, __handle_shader_vs, __handle_shader_fs);
    }

    void Draw(const mat4* projectionMatrix, const mat4* viewMatrix)
    {
        if (__colors.count == 0) return;

        __handle_shader.UseShader();
        __handle_shader.GLBindMatrix4fv("projectionMatrix", 1, projectionMatrix->ptr());
        __handle_shader.GLBindMatrix4fv("viewMatrix", 1, viewMatrix->ptr());

        u32 __vbIndexer = 0;
        for (int i = 0; i < __colors.count; ++i)
        {
            vec4 color = __colors.At(i);
            u16 vertexCount = __verticesCounts.At(i);
            float* vb = __vb.data + __vbIndexer;
            __vbIndexer += vertexCount * 3;

            __handle_shader.GLBind4f("handleColour", color.x, color.y, color.z, color.w);
            glBindVertexArray(__handleVisualVaoId);
            glBindBuffer(GL_ARRAY_BUFFER, __handleVisualVboId);
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)4 * vertexCount * 3, vb, GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        __colors.ResetCount();
        __verticesCounts.ResetCount();
        __vb.ResetCount();
    }

    void DrawSolidDisc(vec3 center, vec3 normal, float radius)
    {
        __colors.PushBack(vec4(1.f, 1.f, 0.f, 1.f));
        __verticesCounts.PushBack(12 * 3);

        vec3 tangent = Normalize(Cross(GM_UP_VECTOR, normal));
        quat q = quat(GM_DEG2RAD * 30.f, normal);
        for(int i = 0; i < 12; ++i)
        {
            vec3 a = center;
            vec3 b = center + tangent * radius;
            tangent = RotateVector(tangent, q);
            vec3 c = center + tangent * radius;

            __vb.PushBack(a.x);
            __vb.PushBack(a.y);
            __vb.PushBack(a.z);
            __vb.PushBack(b.x);
            __vb.PushBack(b.y);
            __vb.PushBack(b.z);
            __vb.PushBack(c.x);
            __vb.PushBack(c.y);
            __vb.PushBack(c.z);
        }
    }

    float DistanceToCircle(vec3 center, float radius)
    {
        if (Engine.gfx->activeGameCamera == nullptr) return 9999.f;
        GameCamera* camera = Engine.gfx->activeGameCamera;
        vec3 tangent = Normalize(Cross(GM_UP_VECTOR, Normalize(camera->calculatedDirection)));
        vec3 right = center + tangent * radius;
        vec3 screenPos = camera->WorldToScreenPoint(center);
        vec3 screenPos2 = camera->WorldToScreenPoint(right);
        float pixelRadius = Magnitude(screenPos - screenPos2);
        vec2 toMouse = screenPos - vec2((float) Input.mousePos.x, (float) Input.mousePos.y);
        return GM_max(Magnitude(toMouse) - pixelRadius, 0.f);
    }

    float GetHandleSize(vec3 position, float sizeInPixels)
    {
        if (Engine.gfx->activeGameCamera == nullptr) return 16.f;

        GameCamera* camera = Engine.gfx->activeGameCamera;
        vec3 camPos = camera->position;
        float distanceFromCamera = camera->WorldPointToCameraDistance(position);
        quat camOrientation = EulerToQuat(camera->rotation * GM_DEG2RAD);
        vec3 screenPos = camera->WorldToScreenPoint(camPos + RotateVector(vec3(distanceFromCamera, 0, 0), camOrientation));
        vec3 screenPos2 = camera->WorldToScreenPoint(camPos + RotateVector(vec3(distanceFromCamera, 0, 1), camOrientation));
        float screenDist = Magnitude(screenPos - screenPos2);
        
        return (sizeInPixels / GM_max(screenDist, 0.0001f));
    }
}