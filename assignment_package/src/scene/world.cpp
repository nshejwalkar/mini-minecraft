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
// Helper Functions
//========================================================

// Get height at (x, z)
int World::getHeight(float x, float z) const {
    return static_cast<int>(evaluateContinentalness(getContinentalnessNoise(x, z)));
}

// Get block type at height
BlockType World::getBlockType(int currentHeight, int maxHeight) const {

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