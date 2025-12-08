#include "world.h"

// Constructor
World::World()
    : noise(SEED) 
{}

//========================================================
// Continentalness Functions
//========================================================

// Evaluate continentalness using spline
float World::evaluateContinentalness(float x) const {
    const auto& first = spline.front();
    const auto& last = spline.back();

    for (size_t i = 0; i < spline.size() - 1; ++i) {
        const auto& p0 = spline[i];
        const auto& p1 = spline[i + 1];

        if (x >= p0.first && x <= p1.first) {
            float normalized = (x - p0.first) / (p1.first - p0.first);
            return p0.second + normalized * (p1.second - p0.second);
        }
    }
    
    return 0.0f;
}

// Get continentalness noise
float World::getContinentalnessNoise(float x, float z) const {
    return noise.fractalPerlinNoise(x, z, CONTINENTALNESS_OCTAVES, CONTINENTALNESS_PERSISTENCE, CONTINENTALNESS_FREQUENCY, CONTINENTALNESS_LACUNARITY);
}

//========================================================
// Biome Functions
//========================================================

// Get temperature noise
float World::getTemperatureNoise(float x, float z) const {
    return noise.perlinNoise(glm::vec2(x + 10000, z + 10000) * TEMP_SCALE);
}

// Get temperature based on Perlin noise
World::Temp World::getTemperature(float x, float z) const {
    // Sample Perlin noise
    float noiseValue = getTemperatureNoise(x, z);
    
    // Map noise value to temperature
    if (noiseValue < -0.2f) {
        return Temp::COLD;
    }
    else if (noiseValue < 0.15f) {
        return Temp::WARM;
    }
    else {
        return Temp::HOT;
    }
}

// Get biome based on temperature
World::Biome World::getBiome(float x, float z) const {
    Temp temp = getTemperature(x, z);
    
    if (temp == Temp::COLD) {
        return Biome::SNOWY_PLAINS;
    }
    else if (temp == Temp::WARM) {
        return Biome::PLAINS;
    }
    else {
        return Biome::DESERT;
    }
}

//========================================================
// Cave Functions
//========================================================

// Check if block at (x, y, z) is in a cave
bool World::isCave(int x, int y, int z) const {
    if (y >= 1 && y <= CAVE_HEIGHT) {
        float caveNoise = getCaveNoise(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
        
        // Taper caves off towards CAVE_HEIGHT
        float taperStart = CAVE_HEIGHT * 0.7f;
        if (y > taperStart) {
            float t = (y - taperStart) / (CAVE_HEIGHT - taperStart);
            float scaledNoise = caveNoise * (1.0f - t);
            return scaledNoise > CAVE_THRESHOLD;
        }
        else {
            return caveNoise > CAVE_THRESHOLD;
        }
    }

    return false;
}

// Get cave block type
BlockType World::getCaveBlockType(int y) const {
    // Under lava height
    if (y < LAVA_HEIGHT) {
        return BlockType::LAVA;
    }

    // Above lava height
    return BlockType::EMPTY;
}

// Get cave noise
float World::getCaveNoise(float x, float y, float z) const {
    return noise.fractalSimplexNoise3D(x, y, z, CAVE_OCTAVES, CAVE_PERSISTENCE, CAVE_FREQUENCY, CAVE_LACUNARITY);
}

//========================================================
// Helper Functions
//========================================================

// Get height at (x, z)
int World::getHeight(float x, float z) const {
    return static_cast<int>(evaluateContinentalness(getContinentalnessNoise(x, z)));
}

// Get block type at height
BlockType World::getBlockType(int currentHeight, int maxHeight, Biome biome) const {
    switch(biome) {
        case Biome::PLAINS:
            return getPlainsBlock(currentHeight, maxHeight);
        case Biome::SNOWY_PLAINS:
            return getSnowyPlainsBlock(currentHeight, maxHeight);
        case Biome::DESERT:
            return getDesertBlock(currentHeight, maxHeight);
    }
    
    return getPlainsBlock(currentHeight, maxHeight);
}

// Plains biome blocks
BlockType World::getPlainsBlock(int currentHeight, int maxHeight) const {
    // Above max height
    if (currentHeight >= maxHeight) {
        if (currentHeight <= WATER_HEIGHT) {
            return BlockType::WATER;
        }
        return BlockType::EMPTY;
    }

    // Underground
    if (currentHeight < maxHeight - 4) {
        return BlockType::STONE;
    }

    // Bedrock 
    if (currentHeight == 0) {
        return BlockType::BEDROCK;
    }

    // Snowcap
    if (maxHeight >= SNOW_HEIGHT) {
        if (currentHeight == maxHeight - 1) {
            return BlockType::SNOW;
        }
        return BlockType::STONE;
    }

    // Mountain
    if (maxHeight >= STONE_HEIGHT) {
        return BlockType::STONE;
    }

    // Sand
    if (maxHeight <= SAND_HEIGHT) {
        return BlockType::SAND;
    }

    // Grassland
    if (currentHeight == maxHeight - 1) {
        return BlockType::GRASS;
    }

    // Grassland underground
    return BlockType::DIRT;
}

// Snowy plains biome blocks
BlockType World::getSnowyPlainsBlock(int currentHeight, int maxHeight) const {
    // Above max height
    if (currentHeight >= maxHeight) {
        if (currentHeight <= WATER_HEIGHT) {
            return BlockType::WATER;
        }
        return BlockType::EMPTY;
    }

    // Underground
    if (currentHeight < maxHeight - 4) {
        return BlockType::STONE;
    }

    // Bedrock 
    if (currentHeight == 0) {
        return BlockType::BEDROCK;
    }

    // Snowcap
    if (maxHeight >= SNOW_HEIGHT) {
        if (currentHeight == maxHeight - 1) {
            return BlockType::SNOW;
        }
        return BlockType::STONE;
    }

    // Mountain
    if (maxHeight >= STONE_HEIGHT) {
        return BlockType::STONE;
    }

    // Sand
    if (maxHeight <= SAND_HEIGHT) {
        return BlockType::SAND;
    }

    // Grassland
    if (currentHeight == maxHeight - 1) {
        return BlockType::SNOWY_GRASS;
    }

    // Grassland underground
    return BlockType::DIRT;
}

// Desert biome blocks
BlockType World::getDesertBlock(int currentHeight, int maxHeight) const {
    // Above max height
    if (currentHeight >= maxHeight) {
        if (currentHeight <= WATER_HEIGHT) {
            return BlockType::WATER;
        }
        return BlockType::EMPTY;
    }

    // Underground
    if (currentHeight < maxHeight - 4) {
        return BlockType::STONE;
    }

    // Bedrock 
    if (currentHeight == 0) {
        return BlockType::BEDROCK;
    }

    // Snowcap
    if (maxHeight >= SNOW_HEIGHT) {
        return BlockType::STONE;
    }

    // Mountain
    if (maxHeight >= STONE_HEIGHT) {
        return BlockType::STONE;
    }

    // Sand
    if (maxHeight <= SAND_HEIGHT) {
        return BlockType::SAND;
    }

    // Grassland
    if (currentHeight == maxHeight - 1) {
        return BlockType::SAND;
    }

    // Grassland underground
    return BlockType::SAND;
}

//========================================================
// Decoration Functions
//========================================================

// Check if cactus can be placed at (x, z)
bool World::validCactusPlacement(int x, int z, World::Biome biome) const {
    if (biome != Biome::DESERT) {
        return false;
    }
    float worleyDist = noise.worleyNoise(static_cast<float>(x), static_cast<float>(z), CACTUS_SCALE);
    return worleyDist < CACTUS_THRESHOLD;
}

// Get height of cactus at (x, z)
int World::getCactusHeight(int x, int z) const {
    float heightNoise = noise.perlinNoise(glm::vec2(x + 5000, z + 5000) * 0.05f);
    int height = 2 + static_cast<int>(((heightNoise + 0.3f) / 0.6f) * 2.0f);
    return glm::clamp(height, 2, 4);
}

// Check if oak tree can be placed at (x, z)
bool World::validOakTreePlacement(int x, int z, World::Biome biome) const {
    if (biome != Biome::PLAINS) {
        return false;
    }

    // Check threshold
    float worleyDist = noise.worleyNoise(static_cast<float>(x), static_cast<float>(z), OAK_TREE_SCALE);
    if (worleyDist >= OAK_TREE_THRESHOLD) {
        return false;
    }
    
    // Secondary noise filter
    float secondaryNoise = noise.perlinNoise(glm::vec2(x + 7000, z + 7000) * 0.03f);
    return secondaryNoise > -0.5f;
}

// Check if birch tree can be placed at (x, z)
bool World::validBirchTreePlacement(int x, int z, World::Biome biome) const {
    if (biome != Biome::SNOWY_PLAINS) {
        return false;
    }

    // Check threshold
    float worleyDist = noise.worleyNoise(static_cast<float>(x), static_cast<float>(z), OAK_TREE_SCALE);
    if (worleyDist >= OAK_TREE_THRESHOLD) {
        return false;
    }
    
    // Secondary noise filter
    float secondaryNoise = noise.perlinNoise(glm::vec2(x + 7000, z + 7000) * 0.03f);
    return secondaryNoise > -0.5f;
}

// Get height of tree at (x, z)
int World::getTreeHeight(int x, int z, int offset) const {
    float heightNoise = noise.perlinNoise(glm::vec2(x + offset, z + offset) * 0.07f);
    int height = 5 + static_cast<int>((heightNoise + 0.3f) / 0.6f * 2.0f);
    return glm::clamp(height, 5, 7);
}

// Place a tree structure in a chunk
void World::placeTree(Chunk* chunk, int localX, int localY, int localZ, int treeHeight, int worldX, int worldZ, BlockType logType, BlockType leavesType) const {
    // Noise functions for random leave placement
    float leaveCornerNoiseA = noise.perlinNoise(glm::vec2(worldX + 8000, worldZ + 8000) * 0.5f);
    float leaveCornerNoiseB = noise.perlinNoise(glm::vec2(worldX + 9000, worldZ + 9000) * 0.5f);
    float leaveCornerNoiseC = noise.perlinNoise(glm::vec2(worldX + 10000, worldZ + 10000) * 0.5f);
    
    // Place trunk
    for (int y = 0; y < treeHeight; ++y) {
        chunk->setLocalBlockAt(localX, localY + y, localZ, logType);
    }
    
    // Layer 1
    chunk->setLocalBlockAt(localX, localY + treeHeight, localZ, leavesType);
    if (localX > 0) chunk->setLocalBlockAt(localX - 1, localY + treeHeight, localZ, leavesType);
    if (localX < 15) chunk->setLocalBlockAt(localX + 1, localY + treeHeight, localZ, leavesType);
    if (localZ > 0) chunk->setLocalBlockAt(localX, localY + treeHeight, localZ - 1, leavesType);
    if (localZ < 15) chunk->setLocalBlockAt(localX, localY + treeHeight, localZ + 1, leavesType);

    // Layer 2
    if (localX > 0) chunk->setLocalBlockAt(localX - 1, localY + treeHeight - 1, localZ, leavesType);
    if (localX < 15) chunk->setLocalBlockAt(localX + 1, localY + treeHeight - 1, localZ, leavesType);
    if (localZ > 0) chunk->setLocalBlockAt(localX, localY + treeHeight - 1, localZ - 1, leavesType);
    if (localZ < 15) chunk->setLocalBlockAt(localX, localY + treeHeight - 1, localZ + 1, leavesType);
    
    // Corners
    int n = glm::clamp(1 + static_cast<int>((leaveCornerNoiseA + 0.3f) / 0.3f), 1, 3);
    n = glm::min(n, 4);
    std::vector<std::pair<int, int>> corners = {
        {-1, -1},
        {-1, 1},
        {1, -1},
        {1, 1}
    };

    // Place corners
    for (int i = 0; i < n; ++i) {
        int offsetX = corners[i].first;
        int offsetZ = corners[i].second;
        if (localX + offsetX >= 0 &&
            localX + offsetX < 16 &&
            localZ + offsetZ >= 0 &&
            localZ + offsetZ < 16)
        {
            chunk->setLocalBlockAt(localX + offsetX, localY + treeHeight - 1, localZ + offsetZ, leavesType);
        }
    }
    
    // Layers 3/4
    for (int layer = 3; layer <= 4; ++layer) {        
        for (int offsetX = -2; offsetX <= 2; ++offsetX) {
            for (int offsetZ = -2; offsetZ <= 2; ++offsetZ) {
                int blockX = localX + offsetX;
                int blockZ = localZ + offsetZ;
                
                // Don't place trees out of chunk bounds
                if (blockX < 0 || blockX >= 16 || blockZ < 0 || blockZ >= 16) {
                    continue;
                }
                
                // Skip corners
                if ((abs(offsetX) == 2 && abs(offsetZ) == 2)) {
                    continue;
                }
                
                chunk->setLocalBlockAt(blockX, localY + treeHeight - layer + 1, blockZ, leavesType);
            }
        }
        
        // Select noise 
        float currNoise;
        if (layer == 3) {
            currNoise = leaveCornerNoiseB;
        }
        else {
            currNoise = leaveCornerNoiseC;
        }

        // Corners
        int n = glm::clamp(static_cast<int>((currNoise + 0.3f) / 0.6f * 5.0f), 0, 4);
        std::vector<std::pair<int, int>> corners = {
            {-2, -2},
            {-2, 2},
            {2, -2},
            {2, 2}
        };
        
        // Place corners
        for (int i = 0; i < n; ++i) {
            int offsetX = corners[i].first;
            int offsetZ = corners[i].second;
            int blockX = localX + offsetX;
            int blockZ = localZ + offsetZ;
            
            if (blockX >= 0 &&
                blockX < 16 &&
                blockZ >= 0 &&
                blockZ < 16)
            {
                chunk->setLocalBlockAt(blockX, localY + treeHeight - layer + 1, blockZ, leavesType);
            }
        }

    }
}
