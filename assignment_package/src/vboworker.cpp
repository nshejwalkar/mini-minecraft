#include "vboworker.h"
#include "debug.h"
#include "scene/terrain.h"
#include "scene/chunk.h"

VBOWorker::VBOWorker(Chunk* chunk, Terrain* terrain, QMutex* mutex)
    : chunk(chunk), terrain(terrain), mutex(mutex)
{}

void VBOWorker::run() {
    // VBO data vectors. two opaque, two transparent
    std::vector<GLuint> idxOpaqueData;
    std::vector<GLuint> idxTransData;
    std::vector<glm::vec4> vertexOpaqueData;
    std::vector<glm::vec4> vertexTransData;

    // Generate data
    chunk->populateAllData(idxOpaqueData,
                           idxTransData,
                           vertexOpaqueData,
                           vertexTransData);

    // initialize VBO Data struct
    Terrain::VBOData vboData;
    vboData.idxOpaqueData = std::move(idxOpaqueData);
    vboData.idxTransData = std::move(idxTransData);
    vboData.vertexOpaqueData = std::move(vertexOpaqueData);
    vboData.vertexTransData = std::move(vertexTransData);
    vboData.chunkMapKey = toKey(chunk->minX, chunk->minZ);

    // same logic as the other worker
    mutex->lock();
    terrain->VBODataList.push_back(std::move(vboData));
    mutex->unlock();
}
