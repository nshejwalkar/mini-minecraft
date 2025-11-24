#include "noise.h"

// Constructor
Noise::Noise(int seed)
    : seed(seed)
{}

// Noise basis function
glm::vec2 Noise::random2(glm::vec2 p) const {
    glm::vec2 offset = p + glm::vec2(seed);
    return glm::fract(glm::sin(glm::vec2(
                          glm::dot(offset, glm::vec2(127.1f, 311.7f)),
                          glm::dot(offset, glm::vec2(269.5f, 183.3f))
                          )) * 43758.5453f);
}

// Surflet
float Noise::surflet(glm::vec2 P, glm::vec2 gridPoint) const {
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1.f - 6.f * pow(distX, 5.f) + 15.f * pow(distX, 4.f) - 10.f * pow(distX, 3.f);
    float tY = 1.f - 6.f * pow(distY, 5.f) + 15.f * pow(distY, 4.f) - 10.f * pow(distY, 3.f);

    glm::vec2 gradient = 2.f * random2(gridPoint) - glm::vec2(1.f);
    glm::vec2 diff = P - gridPoint;

    float height = glm::dot(diff, gradient);

    return height * tX * tY;
}

// Perlin Noise
float Noise::perlinNoise(glm::vec2 uv) const {
    float surfletSum = 0.f;

    for (int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy));
        }
    }

    return surfletSum;
}

// Fractal Perlin Noise
float Noise::fractalPerlinNoise(float x, float y, int octaves, float persistence, float frequency, float lacunarity) const {
    float total = 0.f;
    float freq = frequency;
    float amp = 1.0f;
    float maxAmp = 0.f;

    for (int i = 0; i < octaves; i++) {
        total += perlinNoise(glm::vec2(x * freq, y * freq)) * amp;
        maxAmp += amp;
        freq *= lacunarity;
        amp *= persistence;
    }

    float noise = total / maxAmp;
    return glm::clamp(noise, -0.5f, 0.5f);
}

// Worley Noise
float Noise::worleyNoise(float x, float y, float scale) const {
    glm::vec2 uv = glm::vec2(x, y) * scale;
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0f;
    
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));
            glm::vec2 point = random2(uvInt + neighbor);
            glm::vec2 diff = neighbor + point - uvFract;
            float dist = glm::length(diff);
            minDist = glm::min(minDist, dist);
        }
    }
    
    return minDist;
}