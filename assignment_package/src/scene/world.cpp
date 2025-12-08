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