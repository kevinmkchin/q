#include "Resources.h"

#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <gmath.h>
#include <vertext.h>

#include "FileSystem.h"

static std::unordered_map<std::string, Texture> __gpuLoadedTextures;
static std::unordered_map<std::string, MeshGroup> __gpuLoadedModels;
static std::unordered_map<std::string, vtxt_font> __vtxtLoadedFonts;

ResourceManager Resources;

void Texture::UseTexture() const
{
    if (textureId == 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Resources.defaultMissingTexture.textureId);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
}

bool Texture::TextureCreateFromFile(Texture& texture, const char* textureFilePath, GLenum filteringMode)
{
    if (texture.textureId != 0)
    {
        printf("WARNING: Trying to load a Texture when there is already a texture loaded! Clearing texture first...\n");
        TextureDelete(texture); // TODO(Kevin): Remove probably because we don't want to delete if we load all textures at start and unload at end
    }

    auto textureAlreadyLoaded = __gpuLoadedTextures.find(std::string(textureFilePath));
    if (textureAlreadyLoaded != __gpuLoadedTextures.end())
    {
        texture = textureAlreadyLoaded->second;
        return true;
    }

    BitmapHandle textureHandle;
    ReadImage(textureHandle, textureFilePath);
    if (textureHandle.memory == NULL)
    {
        return false;
    }
    TextureCreateFromBitmap(texture, (unsigned char*)textureHandle.memory, textureHandle.width,
        textureHandle.height, GL_RGBA, (textureHandle.bitDepth == 3 ? GL_RGB : GL_RGBA), filteringMode);
    FreeImage(textureHandle); // texture data has been copied to GPU memory, so we can free image from memory

    texture.filePath = textureFilePath;
    __gpuLoadedTextures[textureFilePath] = texture;

    return true;
}

void Texture::TextureCreateFromBitmap(Texture& texture,
    unsigned char* bitmap,
    u32             bitmapWidth,
    u32             bitmapHeight,
    GLenum          targetFormat,
    GLenum          sourceFormat,
    GLenum          filteringMode)
{
    if (texture.textureId != 0)
    {
        printf("WARNING: Trying to load a Texture when there is already a texture loaded! Clearing texture first...\n");
        TextureDelete(texture);
    }

    texture.width = bitmapWidth;
    texture.height = bitmapHeight;
    texture.format = sourceFormat;

    glGenTextures(1, &texture.textureId);
    glBindTexture(GL_TEXTURE_2D, texture.textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filteringMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filteringMode);
    glTexImage2D(
        GL_TEXTURE_2D,              // texture target type
        0,                          // level-of-detail number n = n-th mipmap reduction image
        targetFormat,               // format of data to store (target): num of color components
        bitmapWidth,                // texture width
        bitmapHeight,               // texture height
        0,                          // must be 0 (legacy)
        sourceFormat,               // format of data being loaded (source)
        GL_UNSIGNED_BYTE,           // data type of the texture data
        bitmap);                    // data
    glGenerateMipmap(GL_TEXTURE_2D);    // generate mip maps automatically
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::TextureDelete(Texture& texture)
{
    if (texture.textureId == 0)
    {
        return;
    }
    glDeleteTextures(1, &texture.textureId);

    auto gpuTextureIter = __gpuLoadedTextures.find(texture.filePath);
    if (gpuTextureIter != __gpuLoadedTextures.end())
    {
        __gpuLoadedTextures.erase(gpuTextureIter);
    }

    texture.textureId = 0;
    texture.width = 0;
    texture.height = 0;
    texture.format = GL_NONE;
}



void Mesh::RenderMesh(GLenum renderMode) const
{
    if (indicesCount == 0) // Early out if index_count == 0, nothing to draw
    {
        printf("WARNING: Attempting to Render a mesh with 0 index count!\n");
        return;
    }

    // Bind VAO, bind VBO, draw elements(indexed draw)
    glBindVertexArray(idVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO);
    glDrawElements(renderMode, indicesCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Mesh::RebindBufferObjects(float* vertices, u32* indices, 
    u32 verticesArrayCount, u32 indicesArrayCount, GLenum drawUsage)
{
    if (idVBO == 0 || idIBO == 0)
    {
        return;
    }

    indicesCount = indicesArrayCount;
    glBindVertexArray(idVAO);
    glBindBuffer(GL_ARRAY_BUFFER, idVBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) 4 * verticesArrayCount, vertices, drawUsage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) 4 * indicesArrayCount, indices, drawUsage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Mesh Mesh::MeshCreate(Mesh& mesh, float* vertices, u32* indices,
    u32 verticesArrayCount, u32 indicesArrayCount,
    u8 positionAttribSize, u8 textureAttribSize, u8 normalAttribSize, GLenum drawUsage)
{
    u8 stride = 0;
    if (textureAttribSize)
    {
        stride += positionAttribSize + textureAttribSize;
        if (normalAttribSize)
        {
            stride += normalAttribSize;
        }
    }

    mesh.indicesCount = indicesArrayCount;

    glGenVertexArrays(1, &mesh.idVAO);
    glBindVertexArray(mesh.idVAO);
    glGenBuffers(1, &mesh.idVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.idVBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) 4 /*bytes cuz float*/ * verticesArrayCount, vertices, drawUsage);
    glVertexAttribPointer(0, positionAttribSize, GL_FLOAT, GL_FALSE, sizeof(float) * stride, nullptr);
    glEnableVertexAttribArray(0);
    if (textureAttribSize > 0)
    {
        glVertexAttribPointer(1, textureAttribSize, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 
            (void*)(sizeof(float) * positionAttribSize));
        glEnableVertexAttribArray(1);
        if (normalAttribSize > 0)
        {
            glVertexAttribPointer(2, normalAttribSize, GL_FLOAT, GL_FALSE, sizeof(float) * stride,
                (void*)(sizeof(float) * ((GLsizeiptr) positionAttribSize + textureAttribSize)));
            glEnableVertexAttribArray(2);
        }
    }

    glGenBuffers(1, &mesh.idIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.idIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) 4 /*bytes cuz uint32*/ * indicesArrayCount, indices, drawUsage);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind the VAO;

    return mesh;
}

void Mesh::MeshDelete(Mesh& mesh)
{
    if (mesh.idIBO != 0)
    {
        glDeleteBuffers(1, &mesh.idIBO);
        mesh.idIBO = 0;
    }
    if (mesh.idVBO != 0)
    {
        glDeleteBuffers(1, &mesh.idVBO);
        mesh.idVBO = 0;
    }
    if (mesh.idVAO != 0)
    {
        glDeleteVertexArrays(1, &mesh.idVAO);
        mesh.idVAO = 0;
    }

    mesh.indicesCount = 0;
}


void MeshGroup::Render()
{
    for (size_t i = 0; i < meshes.size(); ++i)
    {
        u16 materialIndex = meshToTextureTable[i];
        if (materialIndex < textures.size())
        {
            textures[materialIndex].UseTexture();
        }
        meshes[i].RenderMesh();
    }
}

static Mesh __assimpLoadMeshHelper(aiMesh* meshNode)
{
    const u8 vb_entries_per_vertex = 8;
    std::vector<float> vb(meshNode->mNumVertices * vb_entries_per_vertex);
    std::vector<u32> ib(meshNode->mNumFaces * meshNode->mFaces[0].mNumIndices);
    if (meshNode->mTextureCoords[0])
    {
        for (size_t i = 0; i < meshNode->mNumVertices; ++i)
        {
            // mNormals and mVertices are both mNumVertices in size
            size_t v_start_index = i * vb_entries_per_vertex;
            vb[v_start_index] = meshNode->mVertices[i].x;
            vb[v_start_index + 1] = meshNode->mVertices[i].y;
            vb[v_start_index + 2] = meshNode->mVertices[i].z;
            vb[v_start_index + 3] = meshNode->mTextureCoords[0][i].x;
            vb[v_start_index + 4] = meshNode->mTextureCoords[0][i].y;
            vb[v_start_index + 5] = meshNode->mNormals[i].x;
            vb[v_start_index + 6] = meshNode->mNormals[i].y;
            vb[v_start_index + 7] = meshNode->mNormals[i].z;
        }
    }
    else
    {
        for (size_t i = 0; i < meshNode->mNumVertices; ++i)
        {
            size_t v_start_index = i * vb_entries_per_vertex;
            vb[v_start_index] = meshNode->mVertices[i].x;
            vb[v_start_index + 1] = meshNode->mVertices[i].y;
            vb[v_start_index + 2] = meshNode->mVertices[i].z;
            vb[v_start_index + 3] = 0.f;
            vb[v_start_index + 4] = 0.f;
            vb[v_start_index + 5] = meshNode->mNormals[i].x;
            vb[v_start_index + 6] = meshNode->mNormals[i].y;
            vb[v_start_index + 7] = meshNode->mNormals[i].z;
        }
    }

    for (size_t i = 0; i < meshNode->mNumFaces; ++i)
    {
        aiFace face = meshNode->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            ib[i * face.mNumIndices + j] = face.mIndices[j]; // prob sometimes not correct to index ib this way
        }
    }

    Mesh mesh;
    Mesh::MeshCreate(mesh, &vb[0], &ib[0], (u32)vb.size(), (u32)ib.size());
    return mesh;
}

MeshGroup* MeshGroup::MeshGroupAssimpLoad(const char* fileName)
{
    auto modelAlreadyLoaded = __gpuLoadedModels.find(fileName);
    if (modelAlreadyLoaded != __gpuLoadedModels.end())
    {
        return &modelAlreadyLoaded->second;
    }

    Assimp::Importer importer;
    /*  NOTE: To create smooth normals respecting edges sharper than a given angle,
        use importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90) along with
        aiProcess_GenSmoothNormals flag. https://github.com/assimp/assimp/issues/1713

        aiProcess_GenSmoothNormals
        This flag may not be specified together with #aiProcess_GenNormals. There's
        a importer property, #AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE which allows you to
        specify an angle maximum for the normal smoothing algorithm. Normals exceeding
        this limit are not smoothed, resulting in a 'hard' seam between two faces.
        Using a decent angle here (e.g. 80 degrees) results in very good visual
        appearance. To create smooth normals respecting edges sharper than a given angle,
        use importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90) along with
        aiProcess_GenSmoothNormals flag. https://github.com/assimp/assimp/issues/1713

        aiProcess_JoinIdenticalVertices
        is compulsory for indexed drawing. This still works with flat shaded geometry
        because it only joins vertices that are identical in both position and normal.
        e.g. a flat shaded cube will have 24 vertices after joining because each side
        of the cube will have 4 unique vertices and the vertices at the corners will
        not be shared by multiple faces of the cube because they will have different
        normals even though their positions are the same.
    */
    const aiScene* scene = importer.ReadFile(fileName,
        aiProcess_Triangulate
        | aiProcess_GenNormals
        | aiProcess_FlipUVs // textures were fucked cuz this line was missing
        | aiProcess_JoinIdenticalVertices
    );
    if (!scene)
    {
        printf("Model '%s' failed to load: %s\n", fileName, importer.GetErrorString());
        return nullptr;
    }

    MeshGroup meshGroupToLoadTo;

    meshGroupToLoadTo.meshes = std::vector<Mesh>(scene->mNumMeshes);
    meshGroupToLoadTo.textures = std::vector<Texture>(scene->mNumMaterials);
    meshGroupToLoadTo.meshToTextureTable = std::vector<u16>(scene->mNumMeshes);

    // Unpack meshes
    for (size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* mesh_node = scene->mMeshes[i];
        meshGroupToLoadTo.meshes[i] = __assimpLoadMeshHelper(mesh_node);
        meshGroupToLoadTo.meshToTextureTable[i] = mesh_node->mMaterialIndex;
    }

    // Load diffuse textures
    for (size_t i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial* mat = scene->mMaterials[i];
        if (mat->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString path;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                int idx = (int)std::string(path.data).find_last_of("\\");
                std::string texture_file_name = std::string(path.data).substr(idx + 1);
                std::string model_file_directory = std::string(fileName);
                idx = GM_max((int)model_file_directory.find_last_of("/"), (int)model_file_directory.find_last_of("\\"));
                model_file_directory = model_file_directory.substr(0, idx + 1);

                std::string tex_path = model_file_directory + texture_file_name;
                Texture::TextureCreateFromFile(meshGroupToLoadTo.textures[i], tex_path.c_str());
            }
        }
    }

    meshGroupToLoadTo.filePath = fileName;
    __gpuLoadedModels[fileName] = meshGroupToLoadTo;
    return &__gpuLoadedModels[fileName];
}

void MeshGroup::MeshGroupDelete(MeshGroup* const modelToDelete)
{
    for (auto& mesh : modelToDelete->meshes)
    {
        Mesh::MeshDelete(mesh);
    }
    for (auto& texture : modelToDelete->textures)
    {
        Texture::TextureDelete(texture);
    }

    auto gpuModelIter = __gpuLoadedModels.find(modelToDelete->filePath);
    if (gpuModelIter != __gpuLoadedModels.end())
    {
        __gpuLoadedModels.erase(gpuModelIter);
    }
}

Font Font::FontCreateFromFile(const std::string& fontFilePath, u8 fontSize, bool useNearestFiltering)
{
    Font fontToReturn;

    auto fontAlreadyLoaded = __vtxtLoadedFonts.find(fontFilePath);
    if (fontAlreadyLoaded != __vtxtLoadedFonts.end())
    {
        auto textureAlreadyLoaded = __gpuLoadedTextures.find(fontFilePath);
        if (textureAlreadyLoaded != __gpuLoadedTextures.end())
        {
            fontToReturn.ptr = &(fontAlreadyLoaded->second);
            fontToReturn.texture = textureAlreadyLoaded->second;
            return fontToReturn;
        }
        else
        {
            __vtxtLoadedFonts.erase(fontAlreadyLoaded);
        }
    }

    vtxt_font fontHandle;
    Texture fontAtlas;

    BinaryFileHandle fontfile;
    ReadFileBinary(fontfile, fontFilePath.c_str());
    ASSERT(fontfile.memory);
    vtxt_init_font(&fontHandle, (u8*)fontfile.memory, fontSize);
    FreeFileBinary(fontfile);
    Texture::TextureCreateFromBitmap(fontAtlas, fontHandle.font_atlas.pixels, fontHandle.font_atlas.width,
        fontHandle.font_atlas.height, GL_RED, GL_RED, (useNearestFiltering ? GL_NEAREST : GL_LINEAR));
    free(fontHandle.font_atlas.pixels);

    __vtxtLoadedFonts.emplace(std::make_pair(fontFilePath, fontHandle));
    __gpuLoadedTextures.emplace(std::make_pair(fontFilePath, fontAtlas));

    fontToReturn.ptr = &__vtxtLoadedFonts[fontFilePath];
    fontToReturn.texture = fontAtlas;
    fontToReturn.filePath = fontFilePath;
    return fontToReturn;
}

void Font::Delete()
{
    auto vtxtFontIter = __vtxtLoadedFonts.find(filePath);
    if (vtxtFontIter != __vtxtLoadedFonts.end())
    {
        __vtxtLoadedFonts.erase(vtxtFontIter);
    }
    Texture::TextureDelete(texture);
}

