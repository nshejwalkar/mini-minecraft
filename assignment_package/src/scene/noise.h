#pragma once
#include "glm_includes.h"

class Noise
{
private:
    // Random seed
    int seed;

    // Noise basis function
    glm::vec2 random2(glm::vec2 p) const;

    // Noise helper functions
    float surflet(glm::vec2 P, glm::vec2 gridPoint) const;
    float perlinNoise(glm::vec2 uv) const;

public:
    // Constructor
    Noise(int seed);

    // Noise functions
    float fractalPerlinNoise(float x, float y, int octaves, float persistence, float frequency, float lacunarity) const;
    float worleyNoise(float x, float y, float scale) const;
};