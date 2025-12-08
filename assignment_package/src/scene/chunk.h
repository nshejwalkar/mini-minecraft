#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include "drawable.h"
#include <array>
#include <unordered_map>
#include <cstddef>


//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, SAND, LAVA, BEDROCK, SNOWY_GRASS,
    OAK_LOG, OAK_LEAVES, BIRCH_LOG, BIRCH_LEAVES, CACTUS
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// have Chunk inherit from Drawable
class Chunk : public Drawable {
    friend class Terrain;
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;
    std::array<float, 256> m_biomeTemperature;

    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;

    // Get color for a block type
    glm::vec4 getColor(BlockType blockType) const;

    // Get UV for a block
    glm::vec4 getBottomLeftUV(BlockType blockType, bool top = false) const;

public:
    // The coordinates of the chunk's lower-left corner in world space
    int minX, minZ;

    Chunk(OpenGLContext* context, int x, int z);
    BlockType getLocalBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getLocalBlockAt(int x, int y, int z) const;
    void setLocalBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void setBiomeTemperature(unsigned int x, unsigned int z, float temperature);
    float getBiomeTemperature(unsigned int x, unsigned int z) const;
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    
    // Create VBO data
    void createVBOdata() override;

    // two methods that together act the same as createVBOdata
    void populateAllData(std::vector<GLuint>& idxOpaqueData,
                       std::vector<GLuint>& idxTransData,
                       std::vector<glm::vec4>& vertexOpaqueData,
                       std::vector<glm::vec4>& vertexTransData);

    void bufferAllData(std::vector<GLuint>& idxOpaqueData,
                       std::vector<GLuint>& idxTransData,
                       std::vector<glm::vec4>& vertexOpaqueData,
                       std::vector<glm::vec4>& vertexTransData);
};
