#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), m_geomCube(context),
      m_chunkVBOsNeedUpdating(true), mp_context(context)
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
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
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

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram
void Terrain::draw(int minX, int maxX, int minZ, int maxZ, ShaderProgram *shaderProgram) {
    for(int x = minX; x < maxX; x += 16) {
        for(int z = minZ; z < maxZ; z += 16) {
            if(hasChunkAt(x, z)) {
                const uPtr<Chunk> &chunk = getChunkAt(x, z);
                glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, 0, z));
                shaderProgram->setUnifMat4("u_Model", model);
                shaderProgram->setUnifMat4("u_ModelInvTr", glm::inverse(glm::transpose(model)));
                shaderProgram->drawInterleaved(*chunk);
            }
        }
    }
}

// Loads new chunks
void Terrain::loadChunks(glm::vec3 pos) {
    // Player coordinates
    int posX = static_cast<int>(glm::floor(pos.x / 16.f)) * 16;
    int posZ = static_cast<int>(glm::floor(pos.z / 16.f)) * 16;
    
    // Creates chunks from -32 to +32 around the player
    for (int x = -32; x <= 32; x += 16) {
        for (int z = -32; z <= 32; z += 16) {

            // Skip current chunk
            if (x == 0 && z == 0) {
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

                    // Get terrain height and biome
                    int height = m_world.getHeight(worldX, worldZ);
                    BiomeType biome = m_world.getBiome(worldX, worldZ);
                    bool isMountain = (biome == BiomeType::MOUNTAINS);

                    // Fill base with STONE
                    for(int y = 0; y <= 128; ++y) {
                        setGlobalBlockAt(worldX, y, worldZ, STONE);
                    }

                    // Fill height based on biome
                    if (height > 128) {
                        for (int y = 129; y < height; ++y) {
                            // If mountain, STONE
                            if (isMountain) {
                                setGlobalBlockAt(worldX, y, worldZ, STONE);
                            }
                            // If grassland, DIRT
                            else {
                                setGlobalBlockAt(worldX, y, worldZ, DIRT);
                            }
                        }

                        // If mountain above 200, SNOW
                        if (isMountain && height > 200) {
                            setGlobalBlockAt(worldX, height, worldZ, SNOW);
                        }

                        // If mountain below 200, STONE
                        else if (isMountain) {
                            setGlobalBlockAt(worldX, height, worldZ, STONE);
                        }

                        // If grassland, GRASS
                        else {
                            setGlobalBlockAt(worldX, height, worldZ, GRASS);
                        }
                    }

                    // WATER in EMPTY blocks
                    for (int y = 128; y <= 138; ++y) {
                        if (y > height) {
                            setGlobalBlockAt(worldX, y, worldZ, WATER);
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
    // TODO: DELETE THIS LINE WHEN YOU DELETE m_geomCube!
    m_geomCube.createVBOdata();

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

            // Get terrain height and biome
            int height = m_world.getHeight(worldX, worldZ);
            BiomeType biome = m_world.getBiome(worldX, worldZ);
            bool isMountain = (biome == BiomeType::MOUNTAINS);

            // Fill base with STONE
            for(int y = 0; y <= 128; ++y) {
                setGlobalBlockAt(worldX, y, worldZ, STONE);
            }

            // Fill height based on biome
            if (height > 128) {
                for(int y = 129; y < height; ++y) {
                    // If mountain, STONE
                    if (isMountain) {
                        setGlobalBlockAt(worldX, y, worldZ, STONE);
                    }
                    // If grassland, DIRT
                    else {
                        setGlobalBlockAt(worldX, y, worldZ, DIRT);
                    }
                }

                // If mountain above 200, SNOW
                if (isMountain && height > 200) {
                    setGlobalBlockAt(worldX, height, worldZ, SNOW);
                }

                // If mountain below 200, STONE
                else if (isMountain) {
                    setGlobalBlockAt(worldX, height, worldZ, STONE);
                }

                // If grassland, GRASS
                else {
                    setGlobalBlockAt(worldX, height, worldZ, GRASS);
                }
            }

            // WATER in EMPTY blocks
            for (int y = 128; y <= 138; ++y) {
                if (y > height) {
                    setGlobalBlockAt(worldX, y, worldZ, WATER);
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
