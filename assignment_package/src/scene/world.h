#pragma once
#include "noise.h"
#include "chunk.h"
#include <vector>

class World
{
private:

    //========================================================
    // World generation
    //========================================================

    // Noise generator
    Noise noise;

    // Random seed
    static constexpr int SEED = 42;

    // Height constants
    static constexpr int WATER_HEIGHT = 87;
    static constexpr int SNOW_HEIGHT = 150;
    static constexpr int STONE_HEIGHT = 110;
    static constexpr int SAND_HEIGHT = 88;
    static constexpr int CAVE_HEIGHT = 100;
    static constexpr int LAVA_HEIGHT = 25;

    //========================================================
    // Caves
    //========================================================

    // Cave generation constants
    static constexpr float CAVE_THRESHOLD = -0.15f;
    static constexpr float CAVE_FREQUENCY = 0.04f;
    static constexpr int CAVE_OCTAVES = 8;
    static constexpr float CAVE_PERSISTENCE = 0.3f;
    static constexpr float CAVE_LACUNARITY = 6.0f;

    // Cave functions
    float getCaveNoise(float x, float y, float z) const;

    //========================================================
    // Continentalness
    //========================================================

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

    // Continentalness functions
    float evaluateContinentalness(float x) const;
    float getContinentalnessNoise(float x, float z) const;

public:
    // Constructor
    World();

    // Gets the height at (x, z)
    int getHeight(float x, float z) const;

    // Checks if a block at (x, y, z) is in a cave
    bool isCave(int x, int y, int z) const;

    // Gets block types
    BlockType getBlockType(int currentHeight, int maxHeight) const;
    BlockType getCaveBlockType(int y) const;
};