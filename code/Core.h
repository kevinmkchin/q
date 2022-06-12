#pragma once

#include <glew.h>

#include "Common.h"

struct Mesh
{
    /** Stores mesh { VAO, VBO, IBO } info. Handle for VAO on GPU memory
        Holds the ID for the VAO, VBO, IBO in the GPU memory */

    u32  idVAO = 0;
    u32  idVBO = 0;
    u32  idIBO = 0;
    u32  indicesCount = 0;

    /** Binds VAO and draws elements. Bind a shader program and texture
        before calling RenderMesh */
    void RenderMesh(GLenum renderMode = GL_TRIANGLES) const;

    /** Overwrite existing buffer data */
    void RebindBufferObjects(float* vertices,
        u32* indices,
        u32 verticesArrayCount,
        u32 indicesArrayCount,
        GLenum drawUsage = GL_DYNAMIC_DRAW);

    /** Create a Mesh with the given vertices and indices.
        vertex_attrib_size: vertex coords size (e.g. 3 if x y z)
        texture_attrib_size: texture coords size (e.g. 2 if u v)
        draw_usage: affects optimization; GL_STATIC_DRAW buffer data
        only set once, GL_DYNAMIC_DRAW if buffer modified repeatedly */
    static Mesh MeshCreate(Mesh& mesh,
        float* vertices,
        u32* indices,
        u32 verticesArrayCount,
        u32 indicesArrayCount,
        u8 positionAttribSize = 3,  // x y z
        u8 textureAttribSize = 2,   // u v
        u8 normalAttribSize = 3,    // x y z
        GLenum drawUsage = GL_STATIC_DRAW);

    /** Clearing GPU memory: glDeleteBuffers and glDeleteVertexArrays deletes the buffer
        object and vertex array object off the GPU memory. */
    static void MeshDelete(Mesh& mesh);

};
