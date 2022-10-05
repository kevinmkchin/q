#include "LevelGeometry.h"

#include <glew.h>

namespace LevelGeometry
{
    void CreateFaceBatchMeshOnGPU(u32* idVAO, u32* idVBO)
    {
        glGenVertexArrays(1, idVAO);
        glBindVertexArray(*idVAO);
        glGenBuffers(1, idVBO);
        glBindBuffer(GL_ARRAY_BUFFER, *idVBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * (3 + 2)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void DeleteFaceBatchMeshOnGPU(u32 idVAO, u32 idVBO)
    {
        glDeleteBuffers(1, &idVBO);
        glDeleteVertexArrays(1, &idVAO);
    }

    void RebindFaceBatchBufferObject(FaceBatch* faceBatch, u32 sizeInBytes, float* data)
    {
        glBindVertexArray(faceBatch->idVAO);
        glBindBuffer(GL_ARRAY_BUFFER, faceBatch->idVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_DYNAMIC_DRAW);
        faceBatch->verticesCount = sizeInBytes / (sizeof(float) * 8);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        //glBindVertexArray(0);
        // TODO(Kevin): What even is the point of unbinding...I'm going to be careful about always binding before doing anything with VAOs anyways...
    }

    void RenderFaceBatch(FaceBatch faceBatch)
    {
        glBindVertexArray(faceBatch.idVAO);
        glBindBuffer(GL_ARRAY_BUFFER, faceBatch.idVBO);
        faceBatch.facesTexture.UseTexture();
        glDrawArrays(GL_TRIANGLES, 0, faceBatch.verticesCount);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        //glBindVertexArray(0);
    }
}
