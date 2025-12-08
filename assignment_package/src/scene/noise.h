#pragma once
#include "glm_includes.h"
#include <vector>
#include <cstdint>

class Noise
{
private:
    // Random seed
    int seed;

    // Noise helper functions
    float surflet(glm::vec2 P, glm::vec2 gridPoint) const;
    float surflet3D(glm::vec3 P, glm::vec3 gridPoint) const;
    float perlinNoise3D(glm::vec3 uvw) const;

    // OpenSimplex2S constants 
    // https://gist.github.com/KdotJPG/b1270127455a94ac5d19
    static constexpr int64_t PRIME_X = 0x5205402B9270C86F;
    static constexpr int64_t PRIME_Y = 0x598CD327003817B5;
    static constexpr int64_t PRIME_Z = 0x5BCC226E9FA0BACB;
    static constexpr int64_t HASH_MULTIPLIER = 0x53A3F72DEEC546F5;
    static constexpr float ROOT3OVER3 = 0.577350269189626f;
    static constexpr float ROTATE3_ORTHOGONALIZER = -0.21132486540518713f;
    static constexpr float RSQUARED_3D = 3.0f / 4.0f;
    static constexpr int N_GRADS_3D = 256;
    
    // 3D gradient table
    std::vector<float> GRADIENTS_3D;
    
    // Simplex noise helper functions
    void initGradients3D();
    int fastFloor(float x) const;
    float grad3(int64_t seed, int64_t xrvp, int64_t yrvp, int64_t zrvp, float dx, float dy, float dz) const;
    float simplexNoise3DBase(float xr, float yr, float zr) const;

public:
    // Constructor
    Noise(int seed);

    // Noise basis functions
    glm::vec2 random2(glm::vec2 p) const;
    glm::vec3 random3(glm::vec3 p) const;
    
    // Noise functions
    float perlinNoise(glm::vec2 uv) const;
    float fractalPerlinNoise(float x, float y, int octaves, float persistence, float frequency, float lacunarity) const;
    float fractalPerlinNoise3D(float x, float y, float z, int octaves, float persistence, float frequency, float lacunarity) const;
    float worleyNoise(float x, float y, float scale, glm::vec2* outFeaturePoint = nullptr) const;
    
    // Simplex noise functions
    float simplexNoise3D(float x, float y, float z) const;
    float fractalSimplexNoise3D(float x, float y, float z, int octaves, float persistence, float frequency, float lacunarity) const;
};