#include "chunk.h"


Chunk::Chunk(OpenGLContext* context, int x, int z) : Drawable(context), m_blocks(), minX(x), minZ(z), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

// Does bounds checking with at()
BlockType Chunk::getLocalBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getLocalBlockAt(int x, int y, int z) const {
    return getLocalBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setLocalBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
    }
}

void Chunk::createVBOdata() {
    // Store vertex and index data
    std::vector<glm::vec4> vertexData;
    std::vector<GLuint> idxData;
    int numFaces = 0;

    // Iterate over all blocks in a chunk
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 256; ++y) {
            for (int z = 0; z < 16; ++z) {

                // Get the current block type
                BlockType currBlock = getLocalBlockAt(x, y, z);

                // Skip EMPTY
                if (currBlock == EMPTY) {
                    continue;
                }

                // Get neighboring blocks
                BlockType xPos, xNeg, yPos, yNeg, zPos, zNeg;
                xPos = (x < 15)  ? getLocalBlockAt(x + 1, y, z) : (m_neighbors.at(XPOS) ? m_neighbors.at(XPOS)->getLocalBlockAt(0, y, z)  : EMPTY);
                xNeg = (x > 0)   ? getLocalBlockAt(x - 1, y, z) : (m_neighbors.at(XNEG) ? m_neighbors.at(XNEG)->getLocalBlockAt(15, y, z) : EMPTY);
                yPos = (y < 255) ? getLocalBlockAt(x, y + 1, z) : EMPTY;
                yNeg = (y > 0)   ? getLocalBlockAt(x, y - 1, z) : EMPTY;
                zPos = (z < 15)  ? getLocalBlockAt(x, y, z + 1) : (m_neighbors.at(ZPOS) ? m_neighbors.at(ZPOS)->getLocalBlockAt(x, y, 0)  : EMPTY);
                zNeg = (z > 0)   ? getLocalBlockAt(x, y, z - 1) : (m_neighbors.at(ZNEG) ? m_neighbors.at(ZNEG)->getLocalBlockAt(x, y, 15) : EMPTY);
                
                // Get block color and position
                glm::vec4 color = getColor(currBlock);
                glm::vec4 pos(x, y, z, 0);

                // If neighbor EMPTY, add pos/normal/color to vertex data 
                if (xPos == EMPTY) {
                    glm::vec4 normal(1, 0, 0, 0);
                    vertexData.push_back(glm::vec4(1, 0, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 0, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 1, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 1, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    numFaces++;
                }

                if (xNeg == EMPTY) {
                    glm::vec4 normal(-1, 0, 0, 0);
                    vertexData.push_back(glm::vec4(0, 0, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 0, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 1, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 1, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    numFaces++;
                }

                if (yPos == EMPTY) {
                    glm::vec4 normal(0, 1, 0, 0);
                    vertexData.push_back(glm::vec4(0, 1, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 1, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 1, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 1, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    numFaces++;
                }

                if (yNeg == EMPTY) {
                    glm::vec4 normal(0, -1, 0, 0);
                    vertexData.push_back(glm::vec4(0, 0, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 0, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 0, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 0, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    numFaces++;
                }

                if (zPos == EMPTY) {
                    glm::vec4 normal(0, 0, 1, 0);
                    vertexData.push_back(glm::vec4(0, 0, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 0, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 1, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 1, 1, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    numFaces++;
                }

                if (zNeg == EMPTY) {
                    glm::vec4 normal(0, 0, -1, 0);
                    vertexData.push_back(glm::vec4(1, 0, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 0, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(0, 1, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    vertexData.push_back(glm::vec4(1, 1, 0, 1) + pos);
                    vertexData.push_back(normal);
                    vertexData.push_back(color);
                    numFaces++;
                }
            }
        }
    }

    // Generate indices
    int face = 0;
    for (int i = 0; i < numFaces; i++) {
        idxData.push_back(face);
        idxData.push_back(face + 1);
        idxData.push_back(face + 2);
        idxData.push_back(face);
        idxData.push_back(face + 2);
        idxData.push_back(face + 3);
        face += 4;
    }

    // Buffer data
    indexCounts[INDEX] = idxData.size();
    
    generateBuffer(INTERLEAVED);
    bindBuffer(INTERLEAVED);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::vec4), vertexData.data(), GL_STATIC_DRAW);
    
    generateBuffer(INDEX);
    bindBuffer(INDEX);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxData.size() * sizeof(GLuint), idxData.data(), GL_STATIC_DRAW);
}

// Get color for a block type
glm::vec4 Chunk::getColor(BlockType blockType) const {
    switch(blockType) {
        case GRASS:
            return glm::vec4(glm::vec3(95.f, 159.f, 53.f) / 255.f, 1.f);
        case DIRT:
            return glm::vec4(glm::vec3(121.f, 85.f, 58.f) / 255.f, 1.f);
        case STONE:
            return glm::vec4(glm::vec3(0.5f), 1.f);
        case WATER:
            return glm::vec4(glm::vec3(0.f, 0.f, 0.75f), 1.f);
        case SNOW:
            return glm::vec4(glm::vec3(1.f), 1.f);
        case SAND:
            return glm::vec4(glm::vec3(247.f, 233.f, 163.f) / 255.f, 1.f);
        default:
            return glm::vec4(glm::vec3(1.f, 0.f, 1.f), 1.f);
    }
}
