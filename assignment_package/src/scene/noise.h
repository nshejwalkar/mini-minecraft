#pragma once
#include "glm_includes.h"

class Noise
{
private:
    // Random seed
    int seed;

    // Noise basis function
    glm::vec2 random2(glm::vec2 p) const;
    glm::vec3 random3(glm::vec3 p) const;

    // Noise helper functions
    float surflet(glm::vec2 P, glm::vec2 gridPoint) const;
    float perlinNoise(glm::vec2 uv) const;
    float surflet3D(glm::vec3 P, glm::vec3 gridPoint) const;
    float perlinNoise3D(glm::vec3 uvw) const;

public:
    // Constructor
    Noise(int seed);

    // Noise functions
    float fractalPerlinNoise(float x, float y, int octaves, float persistence, float frequency, float lacunarity) const;
    float fractalPerlinNoise3D(float x, float y, float z, int octaves, float persistence, float frequency, float lacunarity) const;
    float worleyNoise(float x, float y, float scale) const;
};