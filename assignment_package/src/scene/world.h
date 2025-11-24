#pragma once
#include "noise.h"
#include "chunk.h"
#include <vector>

class World
{
private:
    // Random seed
    static constexpr int SEED = 42;

    // Block height constants
    static constexpr int WATER_HEIGHT = 87;
    static constexpr int SNOW_HEIGHT = 150;
    static constexpr int STONE_HEIGHT = 110;
    static constexpr int SAND_HEIGHT = 88;

    // Continentalness constants
    static constexpr float CONTINENTALNESS_FREQUENCY = 0.00325f;
    static constexpr int CONTINENTALNESS_OCTAVES = 8;
    static constexpr float CONTINENTALNESS_LACUNARITY = 2.0f;
    static constexpr float CONTINENTALNESS_PERSISTENCE = 0.5f;
    
    // Continentalness spline
    std::vector<std::pair<float, float>> spline = {
        { -0.5f, 50.0f },
        { -0.4f, 70.0f },
        { -0.3f, 80.0f },
        { -0.1f, 90.0f },
        { 0.0f, 95.0f },
        { 0.1f, 100.0f },
        { 0.2f, 130.0f },
        { 0.3f, 180.0f },
        { 0.4f, 220.0f },
        { 0.5f, 250.0f },
    };

    // Noise generator
    Noise noise;

    // Height helper functions
    float evaluateContinentalness(float x) const;
    float getContinentalnessNoise(float x, float z) const;

public:
    // Constructor
    World();

    // Gets the height at (x, z)
    int getHeight(float x, float z) const;

    // Gets block type at current height given max height
    BlockType getBlockType(int currentHeight, int maxHeight) const;
};