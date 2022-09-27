#pragma once

#include <glew.h>

#include "Common.h"


enum FONTENUM : u8
{
    FONT_SourceCodePro,
    FONT_Iosevka
};



struct Texture
{
    /** Handle for texture stored in GPU memory */

    GLuint      textureId = 0;        // ID for the texture in GPU memory
    i32         width = 0;        // Width of the texture
    i32         height = 0;        // Height of the texture
    GLenum      format = GL_NONE;  // format / bitdepth of texture (GL_RGB would be 3 byte bit depth)
    std::string filePath = "";  // TODO(Kevin): remove filePath from Texture

    void UseTexture() const;      // Binds this texture to Texture Unit 1

    /** Loads texture from bitmap; generates a new texture object in GPU mem; store the id
    of the new texture object into texture.texture_id; sets texture parameters; copies texture data
    into the texture object in GPU mem; and generates mip maps automatically.
    filteringMode
    GL_NEAREST : nearest neighbor
    GL_LINEAR : bilinear filtering
    GL_LINEAR_MIPMAP_LINEAR : trilinear filtering */
    static void TextureCreateFromBitmap(Texture& texture,
        unsigned char* bitmap,
        u32             bitmapWidth,
        u32             bitmapHeight,
        GLenum          targetFormat,
        GLenum          sourceFormat,
        GLenum          filteringMode = GL_NEAREST);



    /** Loads texture at file_path; generates a new texture object in GPU mem; stores the id
        of the new texture object into texture.texture_id; sets texture parameters; copies
        texture data into the texture object in GPU mem; and generates mip maps automatically.
        Returns false if something failed. */
    static bool TextureCreateFromFile(Texture& texture, const char* textureFilePath, GLenum filteringMode = GL_NEAREST);

    /** Deletes texture object from GPU memory; resets texture_id, width, height, bit_depth to 0. */
    static void TextureDelete(Texture& texture); // TODO(Kevin): rename to "delete from GPU" or something
};


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


struct MeshGroup
{
    std::vector<Mesh>     meshes;
    std::vector<Texture>  textures;
    std::vector<u16>      meshToTextureTable; // stores index of textures element corresponding to mesh
    std::string           filePath = "";

    void Render();

    /** Load model from disk through Assimp */
    static MeshGroup* MeshGroupAssimpLoad(const char* fileName);

    static void MeshGroupDelete(MeshGroup* const modelToDelete);
};

struct vtxt_font;
struct Font
{
    vtxt_font* ptr = nullptr;
    Texture texture;
    std::string filePath; // TODO(Kevin): remove filePath from Font

    void Delete();

    static Font FontCreateFromFile(const std::string& fontFilePath, u8 fontSize, bool useNearestFiltering = false);
};

struct ResourceManager
{
    Font fonts[16];

    Texture defaultMissingTexture;
};

extern ResourceManager Resources;
