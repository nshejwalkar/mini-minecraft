#include "terrain.h"
#include "blocktypeworker.h"
#include "cube.h"
#include "debug.h"
#include <stdexcept>
#include <iostream>
#include "constants.h"
#include "vboworker.h"
#include <QThreadPool>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_geomCube(context),
      m_chunkVBOsNeedUpdating(true),
    chunksList(), VBODataList(), chunksMutex(), VBODataMutex(),
    mp_context(context)
{}

Terrain::~Terrain() {
    m_geomCube.destroyVBOdata();
}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getGlobalBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getLocalBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                                  static_cast<unsigned int>(y),
                                  static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        // throw std::out_of_range("Coordinates " + std::to_string(x) +
        //                         " " + std::to_string(y) + " " +
        //                         std::to_string(z) + " have no Chunk!");
        // LOG("Coordinates " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " have no Chunk yet!");
        return BlockType::EMPTY;
    }
}

BlockType Terrain::getGlobalBlockAt(glm::vec3 p) const {
    return getGlobalBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

void Terrain::setGlobalBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setLocalBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                           static_cast<unsigned int>(y),
                           static_cast<unsigned int>(z - chunkOrigin.y),
                           t);
        m_chunkVBOsNeedUpdating = true;
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::setGlobalBlockAtUpdate(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setLocalBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                           static_cast<unsigned int>(y),
                           static_cast<unsigned int>(z - chunkOrigin.y),
                           t);

        // Update VBO data
        c->createVBOdata();
        int currX = static_cast<int>(x - chunkOrigin.x);
        int currZ = static_cast<int>(z - chunkOrigin.y);
        if (currX == 0 && hasChunkAt(x - 16, z)) { getChunkAt(x - 16, z)->createVBOdata(); }
        if (currX == 15 && hasChunkAt(x + 16, z)){ getChunkAt(x + 16, z)->createVBOdata(); }
        if (currZ == 0 && hasChunkAt(x, z - 16)) { getChunkAt(x, z - 16)->createVBOdata(); }
        if (currZ == 15 && hasChunkAt(x, z + 16)) { getChunkAt(x, z + 16)->createVBOdata(); }

        m_chunkVBOsNeedUpdating = true;
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

// creates the Chunk (needs to be in here because it needs the GLContext), but does not register or link neighbors.
Chunk* Terrain::createChunkAt(int x, int z) {
    return new Chunk(mp_context, x, z);
}

// assumes the chunk is already registered in m_chunks.
void Terrain::linkChunkNeighbors(Chunk* cPtr) {
    int x = cPtr->minX;
    int z = cPtr->minZ;

    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
}

// When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    // first render opaque
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if(hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);

                if (!chunk->bufGenerated[INTERLEAVED] ||
                    !chunk->indexCounts.count(INDEX) ||
                    chunk->indexCounts.at(INDEX) == 0) {
                    // LOG("INDEX IS PROBABLY EMPTY FOR CHUNK " << x << ", " << z);
                    continue;
                }

                glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, 0, z));
                shaderProgram->setUnifMat4("u_Model", model);
                shaderProgram->setUnifMat4("u_ModelInvTr", glm::inverse(glm::transpose(model)));
                shaderProgram->drawInterleaved(*chunk);
            }
        }
    }

    // // then render transparent
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if(hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);

                if (!chunk->bufGenerated[INTERLEAVED] ||
                    !chunk->indexCounts.count(INDEX) ||
                    chunk->indexCounts.at(INDEX) == 0) {
                    // LOG("INDEX_TRANSPARENT IS PROBABLY EMPTY FOR CHUNK " << x << ", " << z);
                    continue;
                }

                glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, 0, z));
                shaderProgram->setUnifMat4("u_Model", model);
                shaderProgram->setUnifMat4("u_ModelInvTr", glm::inverse(glm::transpose(model)));
                shaderProgram->drawInterleavedTransparent(*chunk);
            }
        }
    }
}

// add chunk to internal data structures. not any vbos
void Terrain::addChunkInternal(int posX, int posZ) {
    // Create new chunk
    Chunk* chunk = instantiateChunkAt(posX, posZ);

    // Generate terrain for this chunk
    for (int i = 0; i < 16; ++i) {
        for (int k = 0; k < 16; ++k) {
            int worldX = posX + i;
            int worldZ = posZ + k;

            // Get height and biome
            int maxHeight = m_world.getHeight(worldX, worldZ);
            World::Biome biome = m_world.getBiome(worldX, worldZ);

            // Fill blocks
            for (int currHeight = 0; currHeight < 256; ++currHeight) {
                BlockType blockType;

                // Cave
                if (m_world.isCave(worldX, currHeight, worldZ)) {
                    blockType = m_world.getCaveBlockType(currHeight);
                }

                // Normal terrain
                else {
                    blockType = m_world.getBlockType(currHeight, maxHeight, biome);
                }

                // Set block
                if (blockType != BlockType::EMPTY) {
                    setGlobalBlockAt(worldX, currHeight, worldZ, blockType);
                }
            }
        }
    }
}

void Terrain::addChunkVBO(int posx, int posz) {
    uPtr<Chunk> &chunk = getChunkAt(posx, posz);
    chunk->createVBOdata();
}

// this does everything. for every tgz around player, spawn blocktypeworkers to add chunks to internal data if doesnt exist.
// if does exist, spawn vboworkers to fill out the interleaved chunks vbos.
// then, drain these data structures in the main thread and draw.
void Terrain::loadSurroundingTGZs(glm::vec3 pos, bool initial) {
    // player's current zone
    int zoneX = static_cast<int>(glm::floor(pos.x / 64.f));
    int zoneZ = static_cast<int>(glm::floor(pos.z / 64.f));

    for(int dx=-RENDER_RADIUS; dx<=RENDER_RADIUS; dx++) {
        for(int dz=-RENDER_RADIUS; dz<=RENDER_RADIUS; dz++) {
            // LOG(zoneX+dx << " " << zoneZ+dz);
            int currZoneX = zoneX+dx;
            int currZoneZ = zoneZ+dz;
            int64_t zoneKey = toKey(currZoneX, currZoneZ);

            // if (m_generatedTerrain.count(zoneKey)) {
            //     LOG("zone " << currZoneX << ", " << currZoneZ << " already exists");
            // }

            // zone doesnt exist in generated terrain -> spawn blocktypeworker per zone to fill in data and register it
            if (!m_generatedTerrain.count(zoneKey)) {
#if USE_TERRAIN_THREADS
                BlockTypeWorker* worker = new BlockTypeWorker(currZoneX, currZoneZ, this, &chunksMutex);
                // LOG("started blocktypeworker for zone " << currZoneX << ", " << currZoneZ);
                QThreadPool::globalInstance()->start(worker);
#else
                {
                BlockTypeWorker worker(currZoneX, currZoneZ, this, &chunksMutex);
                LOG("started *SERIAL* blocktypeworker for zone " << currZoneX << ", " << currZoneZ);
                worker.run();   // same code, just on this thread
                }
#endif
                m_generatedTerrain.insert(zoneKey);
            }

            // zone exists -> spawn vboworkers per chunk to fill in vbo data
            else {
                int zoneOriginX = currZoneX * 64;  // check
                int zoneOriginZ = currZoneZ * 64;
                for(int x = 0; x < 64; x += 16) {
                    for(int z = 0; z < 64; z += 16) {
                        int posX = zoneOriginX + x;
                        int posZ = zoneOriginZ + z;
                        if (hasChunkAt(posX, posZ)) {
                            uPtr<Chunk>& chunk = getChunkAt(posX, posZ);
                            // check if chunk has a vbo. only if regular and transparent are BOTH nonexistent/empty -> doesnt exist
                            if ((!chunk->indexCounts.count(INDEX) || chunk->indexCounts.at(INDEX) == 0) &&
                                (!chunk->indexCounts.count(INDEX_TRANSPARENT) || chunk->indexCounts.at(INDEX_TRANSPARENT) == 0)) {
#if USE_TERRAIN_THREADS
                                VBOWorker* worker = new VBOWorker(chunk.get(), this, &VBODataMutex);
                                LOG("for zone " << currZoneX << ", " << currZoneZ << " started vboworker for chunk at " << posX << ", " << posZ);
                                QThreadPool::globalInstance()->start(worker);
#else
                            {
                                VBOWorker worker(chunk.get(), this, &VBODataMutex);
                                worker.run();   // fills VBODataList immediately. no thread spawned
                            }

#endif
                            }
                        }
                    }
                }
            }
        }
    }

    // member variables chunksList and vboDataList are filled for all chunks in all tgzs. time to drain.
    // if initial, we want to synchronize all threads to make sure all initial zones+chunks load in at same time.
    // besides that, logic is the exact same: add chunks+link neighbors, start vboworkers like before (if not done already), buffer data.
    if (initial) {
        // LOG("INITIAL: finished initial blocktypeworkers. synchronizing...");
        QThreadPool::globalInstance()->waitForDone();  // now all threads are synced

        std::vector<Chunk*> localChunks;  // now work local
        chunksMutex.lock();
        localChunks = std::move(chunksList);
        chunksList.clear();
        chunksMutex.unlock();

        // now go through all the Chunk*s and register+link neighbors
        // LOG("INITIAL: looping thru localChunks");
        for (Chunk* c : localChunks) {
            uPtr<Chunk> cUptr(c);
            m_chunks[toKey(c->minX, c->minZ)] = std::move(cUptr);  // register

            // safe cuz m_chunks is never deleted from
            linkChunkNeighbors(c);

            // same logic as before. fill this->VBODataList
            // Check if chunk needs VBO data: either INDEX doesn't exist/is empty, or it's -1 (uninitialized)
            bool needsVBO = (!c->indexCounts.count(INDEX) || c->indexCounts.at(INDEX) <= 0) &&
                            (!c->indexCounts.count(INDEX_TRANSPARENT) || c->indexCounts.at(INDEX_TRANSPARENT) <= 0);
            if (needsVBO) {
                // LOG("INITIAL: starting vboworker for chunk at " << c->minX << ", " << c->minZ);
#if USE_TERRAIN_THREADS
                VBOWorker* worker = new VBOWorker(c, this, &VBODataMutex);
                QThreadPool::globalInstance()->start(worker);
#else
                {
                    VBOWorker worker(c, this, &VBODataMutex);
                    worker.run();   // fills VBODataList immediately. no thread spawned
                }
                // LOG("jk fading the vboworker. regular createvbodata");
                // c->createVBOdata();
#endif
            }
        }

        // LOG("INITIAL: synchronizing again...");
        QThreadPool::globalInstance()->waitForDone();  // sync again

        std::vector<VBOData> localVBOs;  // local, again
        VBODataMutex.lock();
        localVBOs = std::move(VBODataList);
        VBODataList.clear();
        VBODataMutex.unlock();

        // LOG("INITIAL: looping thru localVBOs...");
        // now run through the vbos (one per chunk) and buffer them. done!
        for (VBOData& vbo : localVBOs) {
            if (m_chunks.count(vbo.chunkMapKey)) {
                uPtr<Chunk>& chunk = m_chunks[vbo.chunkMapKey];

                chunk->bufferAllData(vbo.idxOpaqueData,
                                    vbo.idxTransData,
                                    vbo.vertexOpaqueData,
                                    vbo.vertexTransData);
            }
        }
    }

    // every tick besides the first, has the exact same logic, without the syncs (because we dont want to wait for half loaded chunks)
    else {
        std::vector<Chunk*> localChunks;  // now work local
        chunksMutex.lock();
        localChunks = std::move(chunksList);
        chunksList.clear();
        chunksMutex.unlock();

        // now go through all the Chunk*s and register+link neighbors
        for (Chunk* c : localChunks) {
            uPtr<Chunk> cUptr(c);
            m_chunks[toKey(c->minX, c->minZ)] = std::move(cUptr);  // register

            // safe cuz m_chunks is never deleted from
            linkChunkNeighbors(c);

            // same logic as before. fill this->VBODataList
            // Check if chunk needs VBO data: either INDEX doesn't exist/is empty, or it's -1 (uninitialized)
            bool needsVBO = (!c->indexCounts.count(INDEX) || c->indexCounts.at(INDEX) <= 0) &&
                            (!c->indexCounts.count(INDEX_TRANSPARENT) || c->indexCounts.at(INDEX_TRANSPARENT) <= 0);
            if (needsVBO) {
                // LOG("NOT INITIAL: starting vboworker for chunk at " << c->minX << ", " << c->minZ);
#if USE_TERRAIN_THREADS
                VBOWorker* worker = new VBOWorker(c, this, &VBODataMutex);
                QThreadPool::globalInstance()->start(worker);
#else
                {
                    VBOWorker worker(c, this, &VBODataMutex);
                    worker.run();   // fills VBODataList immediately. no thread spawned
                }
#endif
            }
        }

        std::vector<VBOData> localVBOs;  // local, again
        VBODataMutex.lock();
        localVBOs = std::move(VBODataList);
        VBODataList.clear();
        VBODataMutex.unlock();

        // now run through the vbos (one per chunk) and buffer them. done!
        for (VBOData& vbo : localVBOs) {
            if (m_chunks.count(vbo.chunkMapKey)) {
                uPtr<Chunk>& chunk = m_chunks[vbo.chunkMapKey];

                chunk->bufferAllData(vbo.idxOpaqueData,
                                     vbo.idxTransData,
                                     vbo.vertexOpaqueData,
                                     vbo.vertexTransData);
            }
        }
    }
}


// // Loads the 4x4 chunks and registers the zone
// void Terrain::loadTGZ(int zoneX, int zoneZ, bool initial) {
//     int64_t zoneKey = toKey(zoneX, zoneZ);
//     if (!m_generatedTerrain.count(zoneKey)) {
//         LOG("adding " << zoneX << " " << zoneZ << " to m_genTer");
//         m_generatedTerrain.insert(zoneKey);
//     }

//     int zoneOriginX = zoneX * 64;
//     int zoneOriginZ = zoneZ * 64;

//     // 4x4 chunks
//     for(int x=0; x<64; x+=16) {
//         for(int z=0; z<64; z+=16) {
//             int posX = zoneOriginX+x;
//             int posZ = zoneOriginZ+z;
//             if (hasChunkAt(posX, posZ)) {
//                 continue;
//             }
//             // LOG("adding " << posX << " " << posZ << " to internal and vbo");
//             addChunkInternal(posX, posZ);
//             addChunkVBO(posX, posZ);
//         }
//     }
// }

// Loads new chunks
void Terrain::loadChunks(glm::vec3 pos, bool initial) {
    // Player coordinates
    int posX = static_cast<int>(glm::floor(pos.x / 16.f)) * 16;
    int posZ = static_cast<int>(glm::floor(pos.z / 16.f)) * 16;

    // Creates chunks from -32 to +32 around the player
    for (int x = -32; x <= 32; x += 16) {
        for (int z = -32; z <= 32; z += 16) {

            // Skip current chunk
            if (x == 0 && z == 0 && !initial) {
                continue;
            }

            // Get current chunk offset
            int currX = posX + x;
            int currZ = posZ + z;

            // If chunk already exists, skip
            if (hasChunkAt(currX, currZ)) {
                continue;
            }

            // Create new chunk
            Chunk* chunk = instantiateChunkAt(currX, currZ);

            // Generate terrain for this chunk
            for (int i = 0; i < 16; ++i) {
                for (int k = 0; k < 16; ++k) {
                    int worldX = currX + i;
                    int worldZ = currZ + k;

                    // Get height and biome
                    int maxHeight = m_world.getHeight(worldX, worldZ);
                    World::Biome biome = m_world.getBiome(worldX, worldZ);

                    // Fill blocks
                    for (int currHeight = 0; currHeight < 256; ++currHeight) {
                        BlockType blockType;

                        // Cave
                        if (m_world.isCave(worldX, currHeight, worldZ)) {
                            blockType = m_world.getCaveBlockType(currHeight);
                        }

                        // Normal terrain
                        else {
                            blockType = m_world.getBlockType(currHeight, maxHeight, biome);
                        }

                        // Set block
                        if (blockType != BlockType::EMPTY) {
                            setGlobalBlockAt(worldX, currHeight, worldZ, blockType);
                        }
                    }
                }
            }

            // Create VBO data
            chunk->createVBOdata();
        }
    }
}

void Terrain::CreateTestScene()
{
    // Create the Chunks that will
    // store the blocks for our
    // initial world space
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            instantiateChunkAt(x, z);
        }
    }
    // Tell our existing terrain set that
    // the "generated terrain zone" at (0,0)
    // now exists.
    m_generatedTerrain.insert(toKey(0, 0));

    // Procedurally generate terrain
    for(int x = 0; x < 64; ++x) {
        for(int z = 0; z < 64; ++z) {
            int worldX = x;
            int worldZ = z;

            // Get height and biome
            int maxHeight = m_world.getHeight(worldX, worldZ);
            World::Biome biome = m_world.getBiome(worldX, worldZ);

            // Fill blocks
            for (int currHeight = 0; currHeight < 256; ++currHeight) {
                BlockType blockType;

                // Cave
                if (m_world.isCave(worldX, currHeight, worldZ)) {
                    blockType = m_world.getCaveBlockType(currHeight);
                }

                // Normal terrain
                else {
                    blockType = m_world.getBlockType(currHeight, maxHeight, biome);
                }

                // Set block
                if (blockType != BlockType::EMPTY) {
                    setGlobalBlockAt(worldX, currHeight, worldZ, blockType);
                }
            }
        }
    }

    // Create VBO data for initial chunks
    for(int x = 0; x < 64; x += 16) {
        for(int z = 0; z < 64; z += 16) {
            uPtr<Chunk> &chunk = getChunkAt(x, z);
            chunk->createVBOdata();
        }
    }

}
