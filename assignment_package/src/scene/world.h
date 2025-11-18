#pragma once
#include "noise.h"

enum class BiomeType {
    GRASSLANDS,
    MOUNTAINS
};

class World
{
private:
    // Noise generator
    Noise noise;
    
    // Random seed
    static constexpr int WORLD_SEED = 42;

    // Height helper functions
    float getGrasslandHeight(float x, float z) const;
    float getMountainHeight(float x, float z) const;
    float getBiomeNoise(float x, float z) const;

public:
    // Constructor
    World();

    // Gets the height at (x, z)
    int getHeight(float x, float z) const;

    // Gets the biome at (x, z)
    BiomeType getBiome(float x, float z) const;
};