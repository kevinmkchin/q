#pragma once

/** Game level geometry 
 * 
 * 
 * Treat faces like sprite batching. i.e. group all faces that use the same texture as one omega mesh (AKA FACE BATCHING???)
 * then bind texture and omega mesh once then render all those disparate faces all at once.
 * 
 * 
 * 
 * 
 * 
 * 
 * needs some sort of knowledge about its collider FALSE
 * 
 *  */

#include "Common.h"
#include "Resources.h"

namespace LevelGeometry
{
    struct FaceBatch
    {
        Texture facesTexture;
        u32 idVAO = 0;
        u32 idVBO = 0;
        u32 verticesCount = 0; // if a vertex is a set of 8 floats, how many vertices in this mesh?
    };

    void CreateFaceBatchMeshOnGPU(u32* idVAO, u32* idVBO);

    void DeleteFaceBatchMeshOnGPU(u32 idVAO, u32 idVBO);

    void RebindFaceBatchBufferObject(FaceBatch* faceBatch, u32 sizeInBytes, float* data);

    void RenderFaceBatch(FaceBatch faceBatch);
}
