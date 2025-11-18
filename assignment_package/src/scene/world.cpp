#include "world.h"

// Constructor
World::World()
    : noise(WORLD_SEED) 
{}

//========================================================
// Noise Functions
//========================================================

// Grassland height
float World::getGrasslandHeight(float x, float z) const {
    float scale = 512.0f;
    float worley = noise.worleyNoise(x / scale, z / scale, 5.0f);
    float perlin = noise.fractalPerlinNoise(x / scale, z / scale, 4.0f, 0.5f, 1.0f) * 0.5f + 0.5f;
    float height = worley + perlin * 0.9f;
    return glm::clamp(height * 32.0f + 125.0f, 0.0f, 255.0f);
}

// Mountains height
float World::getMountainHeight(float x, float z) const {
    float scale = 512.0f;
    float perlin = noise.fractalPerlinNoise(x / scale, z / scale, 6.0f, 0.75f, 2.0f) * 0.5f + 0.5f;
    float height = glm::pow(perlin, 4.0f);
    return glm::clamp(height * 192.0f + 200.0f, 0.0f, 255.0f);
}

// Biome noise
float World::getBiomeNoise(float x, float z) const {
    float scale = 512.0f;
    float perlin = noise.fractalPerlinNoise(x / scale, z / scale, 4.0f, 0.5f, 1.0f) * 0.5f + 0.5f;
    return glm::smoothstep(0.25f, 0.75f, perlin);
}

//========================================================
// Helper Functions
//========================================================

// Get height at (x, z)
int World::getHeight(float x, float z) const {
    float t = getBiomeNoise(x, z);
    return glm::mix(getGrasslandHeight(x, z), getMountainHeight(x, z), t);
}

// Get biome at (x, z)
BiomeType World::getBiome(float x, float z) const {
    return getBiomeNoise(x, z) < 0.5f ? BiomeType::GRASSLANDS : BiomeType::MOUNTAINS;
}
