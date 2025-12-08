#include "blocktypeworker.h"
#include "debug.h"
#include "qthread.h"

BlockTypeWorker::BlockTypeWorker(int zoneX, int zoneZ, Terrain* terrain, QMutex* mutex)
    : zoneX(zoneX), zoneZ(zoneZ), terrain(terrain), mutex(mutex)
{}

// does pretty much the same exact thing as Terrain::addChunkInternal
void BlockTypeWorker::run() {
    int zoneOriginX = zoneX * 64;
    int zoneOriginZ = zoneZ * 64;

    // Create chunks
    std::vector<Chunk*> chunks;
    // LOG("looping through the 4x4 chunks");
    for (int x = 0; x < 64; x += 16) {
        for (int z = 0; z < 64; z += 16) {
            int posX = zoneOriginX + x;
            int posZ = zoneOriginZ + z;
            // LOG("thread " << QThread::currentThreadId() << " doing chunk at " << posX << " " << posZ);
            Chunk* chunk = terrain->createChunkAt(posX, posZ);

            // LOG("looping through the 16x16 blocks");
            // go through each 16x16 blocks in chunk
            for (int i = 0; i < 16; ++i) {
                for (int k = 0; k < 16; ++k) {
                    int worldX = posX + i;
                    int worldZ = posZ + k;
                    // LOG("doing blocks at " << worldX << " " << worldZ);

                    int maxHeight = terrain->m_world.getHeight(worldX, worldZ);
                    World::Biome biome = terrain->m_world.getBiome(worldX, worldZ);

                    for (int currHeight = 0; currHeight < 256; ++currHeight) {
                        // LOG("doing blocks at " << worldX << " " << worldZ << " height " << currHeight);
                        BlockType blockType;

                        // cave
                        if (terrain->m_world.isCave(worldX, currHeight, worldZ)) {
                            blockType = terrain->m_world.getCaveBlockType(currHeight);
                        }

                        // normal
                        else {
                            blockType = terrain->m_world.getBlockType(currHeight, maxHeight, biome);
                        }

                        if (blockType != BlockType::EMPTY) {
                            chunk->setLocalBlockAt(static_cast<unsigned int>(i),
                                                   static_cast<unsigned int>(currHeight),
                                                   static_cast<unsigned int>(k),
                                                   blockType);
                            // terrain->setGlobalBlockAt(worldX, currHeight, worldZ, blockType);
                        }
                    }
                }
            }

            // Add to list
            chunks.push_back(chunk);
        }

    }

    // need a mutex here. this worker was launched asynchronously, so other blocktypeworkers could be writing to terrain->chunksList
    mutex->lock();
    for (Chunk* c : chunks) {
        terrain->chunksList.push_back(c);
    }
    mutex->unlock();
}
