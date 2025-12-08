#pragma once
#include "noise.h"
#include "chunk.h"
#include <vector>

class World
{
friend class Terrain;
friend class BlockTypeWorker;
private:

    //========================================================
    // World generation
    //========================================================

    // Noise generator
    Noise noise;

    // Random seed
    static constexpr int SEED = 21;

    // Height constants
    static constexpr int WATER_HEIGHT = 87;
    static constexpr int SNOW_HEIGHT = 150;
    static constexpr int STONE_HEIGHT = 110;
    static constexpr int SAND_HEIGHT = 88;
    static constexpr int CAVE_HEIGHT = 80;
    static constexpr int LAVA_HEIGHT = 25;

    // Biome constants
    enum Biome : unsigned char {
        PLAINS,
        SNOWY_PLAINS,
        DESERT
    };

    enum Temp : unsigned char {
        COLD,
        WARM,
        HOT
    };

    static constexpr float TEMP_SCALE = 0.001f;

    // Decoration constants
    static constexpr float CACTUS_SCALE = 3.0f;
    static constexpr float CACTUS_THRESHOLD = 0.01f;
    static constexpr float OAK_TREE_SCALE = 0.2f;
    static constexpr float OAK_TREE_THRESHOLD = 0.03f;

    //========================================================
    // Caves
    //========================================================

    // Cave generation constants
    static constexpr float CAVE_THRESHOLD = 0.2f;
    static constexpr float CAVE_FREQUENCY = 0.015f;
    static constexpr int CAVE_OCTAVES = 3;
    static constexpr float CAVE_PERSISTENCE = 0.5f;
    static constexpr float CAVE_LACUNARITY = 2.0f;

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
        { -0.5f, 40.0f },
        { -0.4f, 50.0f },
        { -0.3f, 60.0f },
        { -0.2f, 80.0f },
        { -0.1f, 90.0f },
        { 0.0f, 95.0f },
        { 0.1f, 100.0f },
        { 0.2f, 140.0f },
        { 0.3f, 180.0f },
        { 0.4f, 220.0f },
        { 0.5f, 250.0f },
    };

    // Continentalness functions
    float evaluateContinentalness(float x) const;
    float getContinentalnessNoise(float x, float z) const;

    // Biome functions
    Biome getBiome(float x, float z) const;
    Temp getTemperature(float x, float z) const;

    // Per-biome block type functions
    BlockType getPlainsBlock(int currentHeight, int maxHeight) const;
    BlockType getSnowyPlainsBlock(int currentHeight, int maxHeight) const;
    BlockType getDesertBlock(int currentHeight, int maxHeight) const;

public:
    // Constructor
    World();

    // Gets the height at (x, z)
    int getHeight(float x, float z) const;

    // Checks if a block at (x, y, z) is in a cave
    bool isCave(int x, int y, int z) const;

    // Gets block types
    BlockType getBlockType(int currentHeight, int maxHeight, Biome biome) const;
    BlockType getCaveBlockType(int y) const;
    
    // Get temperature noise
    float getTemperatureNoise(float x, float z) const;

    // Decoration functions
    bool validCactusPlacement(int x, int z, World::Biome biome) const;
    bool validOakTreePlacement(int x, int z, World::Biome biome) const;
    bool validBirchTreePlacement(int x, int z, World::Biome biome) const;
    int getCactusHeight(int x, int z) const;
    int getTreeHeight(int x, int z, int offset) const;
    void placeTree(Chunk* chunk, int localX, int localY, int localZ, int treeHeight, int worldX, int worldZ, BlockType logType, BlockType leavesType) const;
};
