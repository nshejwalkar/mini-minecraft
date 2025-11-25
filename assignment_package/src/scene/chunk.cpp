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

inline bool isTransparent(BlockType b) {
    return (b == WATER);
    // return true;
}

void Chunk::createVBOdata() {
    // Store vertex and index data
    std::vector<glm::vec4> vertexOpaqueData;
    std::vector<GLuint> idxOpaqueData;
    int numOpaqueFaces = 0;

    std::vector<glm::vec4> vertexTransData;
    std::vector<GLuint> idxTransData;
    int numTransFaces = 0;

    std::vector<glm::vec4>* vertexData;
    int* numFaces;

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
                
                // Get block color, uv and position
                glm::vec4 color = getColor(currBlock);

                glm::vec4 sideuv_bl = getBottomLeftUV(currBlock, false);
                glm::vec4 sideuv_tl = glm::vec4(0,1,0,0)/16.f + sideuv_bl;
                glm::vec4 sideuv_br = glm::vec4(1,0,0,0)/16.f + sideuv_bl;
                glm::vec4 sideuv_tr = glm::vec4(1,1,0,0)/16.f + sideuv_bl;

                glm::vec4 topuv_bl = getBottomLeftUV(currBlock, true);
                glm::vec4 topuv_tl = glm::vec4(0,1,0,0)/16.f + topuv_bl;
                glm::vec4 topuv_br = glm::vec4(1,0,0,0)/16.f + topuv_bl;
                glm::vec4 topuv_tr = glm::vec4(1,1,0,0)/16.f + topuv_bl;

                glm::vec4 pos(x, y, z, 0);

                vertexData = isTransparent(currBlock) ? &vertexTransData : &vertexOpaqueData;
                numFaces = isTransparent(currBlock) ? &numTransFaces : &numOpaqueFaces;

                // If neighbor EMPTY, add pos/normal/color to vertex data
                // transparent too
                if (xPos == EMPTY) {
                    glm::vec4 normal(1, 0, 0, 0);
                    vertexData->push_back(glm::vec4(1, 0, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_bl);
                    vertexData->push_back(glm::vec4(1, 0, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_br);
                    vertexData->push_back(glm::vec4(1, 1, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tr);
                    vertexData->push_back(glm::vec4(1, 1, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tl);
                    (*numFaces)++;
                }

                if (xNeg == EMPTY) {
                    glm::vec4 normal(-1, 0, 0, 0);
                    vertexData->push_back(glm::vec4(0, 0, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_bl);
                    vertexData->push_back(glm::vec4(0, 0, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_br);
                    vertexData->push_back(glm::vec4(0, 1, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tr);
                    vertexData->push_back(glm::vec4(0, 1, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tl);
                    (*numFaces)++;
                }

                if (yPos == EMPTY) {
                    glm::vec4 normal(0, 1, 0, 0);
                    vertexData->push_back(glm::vec4(0, 1, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(topuv_bl);
                    vertexData->push_back(glm::vec4(1, 1, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(topuv_br);
                    vertexData->push_back(glm::vec4(1, 1, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(topuv_tr);
                    vertexData->push_back(glm::vec4(0, 1, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(topuv_tl);
                    (*numFaces)++;
                }

                if (yNeg == EMPTY) {
                    glm::vec4 normal(0, -1, 0, 0);
                    vertexData->push_back(glm::vec4(0, 0, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_bl);
                    vertexData->push_back(glm::vec4(1, 0, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_br);
                    vertexData->push_back(glm::vec4(1, 0, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tr);
                    vertexData->push_back(glm::vec4(0, 0, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tl);
                    (*numFaces)++;
                }

                if (zPos == EMPTY) {
                    glm::vec4 normal(0, 0, 1, 0);
                    vertexData->push_back(glm::vec4(0, 0, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_bl);
                    vertexData->push_back(glm::vec4(1, 0, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_br);
                    vertexData->push_back(glm::vec4(1, 1, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tr);
                    vertexData->push_back(glm::vec4(0, 1, 1, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tl);
                    (*numFaces)++;
                }

                if (zNeg == EMPTY) {
                    glm::vec4 normal(0, 0, -1, 0);
                    vertexData->push_back(glm::vec4(1, 0, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_bl);
                    vertexData->push_back(glm::vec4(0, 0, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_br);
                    vertexData->push_back(glm::vec4(0, 1, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tr);
                    vertexData->push_back(glm::vec4(1, 1, 0, 1) + pos);
                    vertexData->push_back(normal);
                    vertexData->push_back(color);
                    vertexData->push_back(sideuv_tl);
                    (*numFaces)++;
                }
            }
        }
    }

    // Generate indices for both opaque and transparent
    int faceO = 0;
    for (int i = 0; i < numOpaqueFaces; i++) {
        idxOpaqueData.push_back(faceO);
        idxOpaqueData.push_back(faceO + 1);
        idxOpaqueData.push_back(faceO + 2);
        idxOpaqueData.push_back(faceO);
        idxOpaqueData.push_back(faceO + 2);
        idxOpaqueData.push_back(faceO + 3);
        faceO += 4;
    }

    int faceT = 0;
    for (int i = 0; i < numTransFaces; i++) {
        idxTransData.push_back(faceT);
        idxTransData.push_back(faceT + 1);
        idxTransData.push_back(faceT + 2);
        idxTransData.push_back(faceT);
        idxTransData.push_back(faceT + 2);
        idxTransData.push_back(faceT + 3);
        faceT += 4;
    }

    // Buffer data
    indexCounts[INDEX] = idxOpaqueData.size();
    indexCounts[INDEX_TRANSPARENT] = idxTransData.size();
    
    // generate all buffers
    generateBuffer(INTERLEAVED);
    bindBuffer(INTERLEAVED);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vertexOpaqueData.size() * sizeof(glm::vec4), vertexOpaqueData.data(), GL_STATIC_DRAW);
    
    generateBuffer(INDEX);
    bindBuffer(INDEX);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxOpaqueData.size() * sizeof(GLuint), idxOpaqueData.data(), GL_STATIC_DRAW);

    generateBuffer(INTERLEAVED_TRANSPARENT);
    bindBuffer(INTERLEAVED_TRANSPARENT);
    mp_context->glBufferData(GL_ARRAY_BUFFER, vertexTransData.size() * sizeof(glm::vec4), vertexTransData.data(), GL_STATIC_DRAW);

    generateBuffer(INDEX_TRANSPARENT);
    bindBuffer(INDEX_TRANSPARENT);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxTransData.size() * sizeof(GLuint), idxTransData.data(), GL_STATIC_DRAW);
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
            return glm::vec4(glm::vec3(0.f, 0.f, 0.75f), 0.5f);
        case SNOW:
            return glm::vec4(glm::vec3(1.f), 1.f);
        case SAND:
            return glm::vec4(glm::vec3(247.f, 233.f, 163.f) / 255.f, 1.f);
        case LAVA:
            return glm::vec4(glm::vec3(1.f, 0.25f, 0.f), 1.f);
        case BEDROCK:
            return glm::vec4(glm::vec3(0.1f, 0.1f, 0.1f), 1.f);
        default:
            return glm::vec4(glm::vec3(1.f, 0.f, 1.f), 1.f);
    }
}

glm::vec4 Chunk::getBottomLeftUV(BlockType blockType, bool top) const {
    bool anim = (blockType == WATER || blockType == LAVA);
    float flag = anim ? 1.f : 0.f;  // animated flag fits into uv.z
    // float flag = 0.f;

    switch(blockType) {
        case GRASS:
            if (top) return glm::vec4(glm::vec2(8.f, 13.f) / 16.f, flag, 0.f);
            else     return glm::vec4(glm::vec2(3.f, 15.f) / 16.f, flag, 0.f);
        case DIRT:
            return glm::vec4(glm::vec2(2.f, 15.f) / 16.f, flag, 0.f);
        case STONE:
            return glm::vec4(glm::vec2(1.f, 15.f) / 16.f, flag, 0.f);
        case WATER:
            return glm::vec4(glm::vec2(13.f, 3.f) / 16.f, flag, 0.f);
        case LAVA:
            return glm::vec4(glm::vec2(13.f, 1.f) / 16.f, flag, 0.f);
        case BEDROCK:
            return glm::vec4(glm::vec2(1.f, 14.f) / 16.f, flag, 0.f);
        case SNOW:
            return glm::vec4(glm::vec2(2.f, 11.f) / 16.f, flag, 0.f);
        case SAND:
            return glm::vec4(glm::vec2(2.f, 14.f) / 16.f, flag, 0.f);
        default:
            return glm::vec4(0,0,0,0);
        }
}
