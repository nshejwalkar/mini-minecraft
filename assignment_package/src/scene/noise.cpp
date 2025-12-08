#include "noise.h"

// Constructor
Noise::Noise(int seed)
    : seed(seed)
{
    initGradients3D();
}

// 2D noise basis function
glm::vec2 Noise::random2(glm::vec2 p) const {
    glm::vec2 offset = p + glm::vec2(seed);
    return glm::fract(glm::sin(glm::vec2(
                          glm::dot(offset, glm::vec2(127.1f, 311.7f)),
                          glm::dot(offset, glm::vec2(269.5f, 183.3f))
                          )) * 43758.5453f);
}

// 3D noise basis function
glm::vec3 Noise::random3(glm::vec3 p) const {
    glm::vec3 offset = p + glm::vec3(seed);
    return glm::fract(glm::sin(glm::vec3(
                          glm::dot(offset, glm::vec3(127.1f, 311.7f, 74.7f)),
                          glm::dot(offset, glm::vec3(269.5f, 183.3f, 246.1f)),
                          glm::dot(offset, glm::vec3(113.5f, 271.9f, 124.6f))
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

// 3D Surflet
float Noise::surflet3D(glm::vec3 P, glm::vec3 gridPoint) const {
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float distZ = abs(P.z - gridPoint.z);
    float tX = 1.f - 6.f * pow(distX, 5.f) + 15.f * pow(distX, 4.f) - 10.f * pow(distX, 3.f);
    float tY = 1.f - 6.f * pow(distY, 5.f) + 15.f * pow(distY, 4.f) - 10.f * pow(distY, 3.f);
    float tZ = 1.f - 6.f * pow(distZ, 5.f) + 15.f * pow(distZ, 4.f) - 10.f * pow(distZ, 3.f);

    glm::vec3 gradient = 2.f * random3(gridPoint) - glm::vec3(1.f);
    glm::vec3 diff = P - gridPoint;

    float height = glm::dot(diff, gradient);

    return height * tX * tY * tZ;
}

// 2D Perlin Noise
float Noise::perlinNoise(glm::vec2 uv) const {
    float surfletSum = 0.f;

    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy));
        }
    }

    return surfletSum;
}

// 3D Perlin Noise
float Noise::perlinNoise3D(glm::vec3 uvw) const {
    float surfletSum = 0.f;

    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            for (int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet3D(uvw, glm::floor(uvw) + glm::vec3(dx, dy, dz));
            }
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

// 3D Fractal Perlin Noise
float Noise::fractalPerlinNoise3D(float x, float y, float z, int octaves, float persistence, float frequency, float lacunarity) const {
    float total = 0.f;
    float freq = frequency;
    float amp = 1.0f;
    float maxAmp = 0.f;

    for (int i = 0; i < octaves; i++) {
        total += perlinNoise3D(glm::vec3(x * freq, y * freq, z * freq)) * amp;
        maxAmp += amp;
        freq *= lacunarity;
        amp *= persistence;
    }

    float noise = total / maxAmp;
    return noise;
}

// Worley Noise
float Noise::worleyNoise(float x, float y, float scale, glm::vec2* outFeaturePoint) const {
    glm::vec2 uv = glm::vec2(x, y) * scale;
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0f;
    glm::vec2 cellId = uvInt;
    glm::vec2 featurePoint = random2(uvInt);
    
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y));
            glm::vec2 point = random2(uvInt + neighbor);
            glm::vec2 diff = neighbor + point - uvFract;
            float dist = glm::length(diff);
            if (dist < minDist) {
                minDist = dist;
                cellId = uvInt + neighbor;
                featurePoint = point;
            }
        }
    }
    
    if (outFeaturePoint != nullptr) {
        *outFeaturePoint = (cellId + featurePoint) / scale;
    }
    
    return minDist;
}

//========================================================
// Simplex Noise Implementation
//
// The following is a C++ adaptation of the OpenSimplex2S
// algorithm by KdotJPG:
// https://gist.github.com/KdotJPG/b1270127455a94ac5d19
//========================================================

// Initialize 3D gradient table:
// This function sets up a large pre-computed table of carefully
// chosen gradient vectors for OpenSimplex2S. Unlike typical simplex
// noise (which uses 12 gradients), this implementation uses 48
// specialized gradients to reduce grid artifacts and improve
// isotropy (uniformity in all directions). Pre-computing these
// normalized gradients also improves runtime efficiency by avoiding
// repeated normalization calculations during noise generation.
void Noise::initGradients3D() {
    const float NORMALIZER_3D = 0.2781926117527186f;
    
    const float grad3_base[] = {
        2.22474487139f,       2.22474487139f,      -1.0f,                 0.0f,
        2.22474487139f,       2.22474487139f,       1.0f,                 0.0f,
        3.0862664687972017f,  1.1721513422464978f,  0.0f,                 0.0f,
        1.1721513422464978f,  3.0862664687972017f,  0.0f,                 0.0f,
        -2.22474487139f,       2.22474487139f,      -1.0f,                 0.0f,
        -2.22474487139f,       2.22474487139f,       1.0f,                 0.0f,
        -1.1721513422464978f,  3.0862664687972017f,  0.0f,                 0.0f,
        -3.0862664687972017f,  1.1721513422464978f,  0.0f,                 0.0f,
        -1.0f,                -2.22474487139f,      -2.22474487139f,       0.0f,
        1.0f,                -2.22474487139f,      -2.22474487139f,       0.0f,
        0.0f,                -3.0862664687972017f, -1.1721513422464978f,  0.0f,
        0.0f,                -1.1721513422464978f, -3.0862664687972017f,  0.0f,
        -1.0f,                -2.22474487139f,       2.22474487139f,       0.0f,
        1.0f,                -2.22474487139f,       2.22474487139f,       0.0f,
        0.0f,                -1.1721513422464978f,  3.0862664687972017f,  0.0f,
        0.0f,                -3.0862664687972017f,  1.1721513422464978f,  0.0f,
        -2.22474487139f,      -2.22474487139f,      -1.0f,                 0.0f,
        -2.22474487139f,      -2.22474487139f,       1.0f,                 0.0f,
        -3.0862664687972017f, -1.1721513422464978f,  0.0f,                 0.0f,
        -1.1721513422464978f, -3.0862664687972017f,  0.0f,                 0.0f,
        -2.22474487139f,      -1.0f,                -2.22474487139f,       0.0f,
        -2.22474487139f,       1.0f,                -2.22474487139f,       0.0f,
        -1.1721513422464978f,  0.0f,                -3.0862664687972017f,  0.0f,
        -3.0862664687972017f,  0.0f,                -1.1721513422464978f,  0.0f,
        -2.22474487139f,      -1.0f,                 2.22474487139f,       0.0f,
        -2.22474487139f,       1.0f,                 2.22474487139f,       0.0f,
        -3.0862664687972017f,  0.0f,                 1.1721513422464978f,  0.0f,
        -1.1721513422464978f,  0.0f,                 3.0862664687972017f,  0.0f,
        -1.0f,                 2.22474487139f,      -2.22474487139f,       0.0f,
        1.0f,                 2.22474487139f,      -2.22474487139f,       0.0f,
        0.0f,                 1.1721513422464978f, -3.0862664687972017f,  0.0f,
        0.0f,                 3.0862664687972017f, -1.1721513422464978f,  0.0f,
        -1.0f,                 2.22474487139f,       2.22474487139f,       0.0f,
        1.0f,                 2.22474487139f,       2.22474487139f,       0.0f,
        0.0f,                 3.0862664687972017f,  1.1721513422464978f,  0.0f,
        0.0f,                 1.1721513422464978f,  3.0862664687972017f,  0.0f,
        2.22474487139f,      -2.22474487139f,      -1.0f,                 0.0f,
        2.22474487139f,      -2.22474487139f,       1.0f,                 0.0f,
        1.1721513422464978f, -3.0862664687972017f,  0.0f,                 0.0f,
        3.0862664687972017f, -1.1721513422464978f,  0.0f,                 0.0f,
        2.22474487139f,      -1.0f,                -2.22474487139f,       0.0f,
        2.22474487139f,       1.0f,                -2.22474487139f,       0.0f,
        3.0862664687972017f,  0.0f,                -1.1721513422464978f,  0.0f,
        1.1721513422464978f,  0.0f,                -3.0862664687972017f,  0.0f,
        2.22474487139f,      -1.0f,                 2.22474487139f,       0.0f,
        2.22474487139f,       1.0f,                 2.22474487139f,       0.0f,
        1.1721513422464978f,  0.0f,                 3.0862664687972017f,  0.0f,
        3.0862664687972017f,  0.0f,                 1.1721513422464978f,  0.0f,
    };
    
    const int grad3_base_size = sizeof(grad3_base) / sizeof(grad3_base[0]);
    
    // Resize and fill gradient table
    GRADIENTS_3D.resize(N_GRADS_3D * 4);
    int j = 0;
    for (int i = 0; i < N_GRADS_3D * 4; i++) {
        GRADIENTS_3D[i] = grad3_base[j] / NORMALIZER_3D;
        j++;
        if (j >= grad3_base_size) {
            j = 0;
        }
    }
}

// Fast floor function:
// This implements a faster alternative to std::floor() for negative
// numbers, which is critical for noise generation performance.
// Standard floor() can be slow due to rounding mode handling, whereas
// this bitwise approach correctly handles negative values with minimal
// overhead. In practice, we notice that this function significantly
// speeds up noise calculations that involve frequent flooring of
// floating-point coordinates.
int Noise::fastFloor(float x) const {
    int xi = static_cast<int>(x);
    return x < xi ? xi - 1 : xi;
}

// 3D gradient function:
// This function computes the dot product between a random gradient vector
// and the distance vector. It uses the seed and lattice point coordinates
//to generate a hash value that maps to one of the 48 pre-computed gradients.
// The result represents the contribution of this lattice point to the final
// noise value at the sample position.
float Noise::grad3(int64_t seed, int64_t xrvp, int64_t yrvp, int64_t zrvp, float dx, float dy, float dz) const {
    // Create a unique hash for this point
    int64_t hash = (seed ^ xrvp) ^ (yrvp ^ zrvp);
    hash = static_cast<int64_t>(static_cast<uint64_t>(hash) * static_cast<uint64_t>(HASH_MULTIPLIER));
    hash ^= hash >> (64 - 8 + 2);
    
    // Extract gradient index
    int gi = static_cast<int>(hash & ((N_GRADS_3D - 1) << 2));
    
    // Return the dot product, multiply gradient components by distance vector and sum
    return GRADIENTS_3D[gi | 0] * dx + GRADIENTS_3D[gi | 1] * dy + GRADIENTS_3D[gi | 2] * dz;
}

// 3D OpenSimplex2S noise:
// This is the main function being used for generating 3D simplex noise at a
// given coordinate (x, y, z). This function first re-orients the cubic lattice by
// applying a rotation transformation to the input coordinates, which helps eliminate
// directional artifacts. The purpose of this rotation is to ensure that the noise
// appears uniform from all viewing angles, rather than showing obvious grid alignment
// along the axes.
float Noise::simplexNoise3D(float x, float y, float z) const {
    // Re-orient the cubic lattices without skewing to reduce directional bias
    float xy = x + y;
    float s2 = xy * ROTATE3_ORTHOGONALIZER;
    float zz = z * ROOT3OVER3;
    float xr = x + s2 + zz;
    float yr = y + s2 + zz;
    float zr = xy * -ROOT3OVER3 + zz;
    
    return simplexNoise3DBase(xr, yr, zr);
}

// Base 3D simplex noise implementation:
// This is the core algorithm that computes simplex noise. To do so,
// it identifies which vertices are within range, computes radial falloff functions,
// and accumulates weighted gradient contributions. Compared to perlin noise,
// simplex noise has fewer directional artifacts and smoother interpolation, making it
// ideal for 3D cave generation where natural-looking structures are much more preferred.
// In practice, we noticed that perlin noise often produced visible artifacts and 
// was less efficient in 3D scenarios.
float Noise::simplexNoise3DBase(float xr, float yr, float zr) const {
    // Get base points and offsets:
    // The algorithm starts by finding the integer lattice cell containing
    // our sample point and computes fractional offsets within that cell.
    int xrb = fastFloor(xr);
    int yrb = fastFloor(yr);
    int zrb = fastFloor(zr);
    float xi = xr - xrb;
    float yi = yr - yrb;
    float zi = zr - zrb;
    
    // Prime pre-multiplication for hash:
    // Next, we multiply cell coordinates by large primes to spread out hash
    // values and reduce collisions when combining coordinates.
    int64_t xrbp = static_cast<int64_t>(xrb) * PRIME_X;
    int64_t yrbp = static_cast<int64_t>(yrb) * PRIME_Y;
    int64_t zrbp = static_cast<int64_t>(zrb) * PRIME_Z;
    
    // Altered seed for second vertex
    int64_t seed2 = seed ^ static_cast<int64_t>(0xAD2A8B2D6942D635ULL);
    
    // Masks:
    // Then, the following three masks determine which side of the 0.5 division
    // our point falls on in each dimension, which is then used for vertex
    // selection
    int xNMask = static_cast<int>(-0.5f - xi);
    int yNMask = static_cast<int>(-0.5f - yi);
    int zNMask = static_cast<int>(-0.5f - zi);
    
    // First vertex:
    // We compute distance to the nearest required vertex and its falloff value.
    // The falloff ensures smooth derivatives and continuity at vertex boundaries.
    float x0 = xi + xNMask;
    float y0 = yi + yNMask;
    float z0 = zi + zNMask;
    float a0 = RSQUARED_3D - x0 * x0 - y0 * y0 - z0 * z0;
    float value = (a0 * a0) * (a0 * a0) * 
                  grad3(seed,
                       xrbp + (xNMask & PRIME_X),
                       yrbp + (yNMask & PRIME_Y),
                       zrbp + (zNMask & PRIME_Z),
                       x0, y0, z0);
    
    // Second vertex:
    // This vertex exists at the center of the simplex structure, it's
    // used for continuity and smoothness in the noise function.
    float x1 = xi - 0.5f;
    float y1 = yi - 0.5f;
    float z1 = zi - 0.5f;
    float a1 = RSQUARED_3D - x1 * x1 - y1 * y1 - z1 * z1;
    value += (a1 * a1) * (a1 * a1) * 
             grad3(seed2,
                  xrbp + PRIME_X,
                  yrbp + PRIME_Y,
                  zrbp + PRIME_Z,
                  x1, y1, z1);
    
    // Pre-compute flip masks:
    // Pre-computing the bit manipulation results is used to efficiently
    // and calculate distances to optional vertices. The "flip masks"
    // implemented here help determine which additional vertices might
    // contribute based on the sample point's position.
    float xAFlipMask0 = ((xNMask | 1) << 1) * x1;
    float yAFlipMask0 = ((yNMask | 1) << 1) * y1;
    float zAFlipMask0 = ((zNMask | 1) << 1) * z1;
    float xAFlipMask1 = (-2 - (xNMask << 2)) * x1 - 1.0f;
    float yAFlipMask1 = (-2 - (yNMask << 2)) * y1 - 1.0f;
    float zAFlipMask1 = (-2 - (zNMask << 2)) * z1 - 1.0f;
    
    // Optional vertices:
    // The following blocks test up to 11 additional vertices for
    // potential contributions. Each vertex is evaluated if its falloff
    // is positive (for efficiency). If so, its gradient contribution
    // is calculated and added to the total value.
    
    bool skip5 = false;

    // Vertex 2: Displaced in X direction from first vertex
    float a2 = xAFlipMask0 + a0;
    if (a2 > 0) {
        float x2 = x0 - (xNMask | 1);
        float y2 = y0;
        float z2 = z0;
        value += (a2 * a2) * (a2 * a2) * 
                 grad3(seed,
                      xrbp + (~xNMask & PRIME_X),
                      yrbp + (yNMask & PRIME_Y),
                      zrbp + (zNMask & PRIME_Z),
                      x2, y2, z2);
    }
    else {
        // Vertex 3: Displaced in Y and Z directions (only tested if vertex 2 doesn't contribute)
        float a3 = yAFlipMask0 + zAFlipMask0 + a0;
        if (a3 > 0) {
            float x3 = x0;
            float y3 = y0 - (yNMask | 1);
            float z3 = z0 - (zNMask | 1);
            value += (a3 * a3) * (a3 * a3) * 
                     grad3(seed,
                          xrbp + (xNMask & PRIME_X),
                          yrbp + (~yNMask & PRIME_Y),
                          zrbp + (~zNMask & PRIME_Z),
                          x3, y3, z3);
        }
        
        // Vertex 4: Displaced in X direction from second vertex
        float a4 = xAFlipMask1 + a1;
        if (a4 > 0) {
            float x4 = (xNMask | 1) + x1;
            float y4 = y1;
            float z4 = z1;
            value += (a4 * a4) * (a4 * a4) * 
                     grad3(seed2,
                          xrbp + (xNMask & (PRIME_X * 2)),
                          yrbp + PRIME_Y,
                          zrbp + PRIME_Z,
                          x4, y4, z4);
            skip5 = true; // Skip vertex 5 if vertex 4 contributes
        }
    }
    
    bool skip9 = false;

    // Vertex 6: Displaced in Y direction from first vertex
    float a6 = yAFlipMask0 + a0;
    if (a6 > 0) {
        float x6 = x0;
        float y6 = y0 - (yNMask | 1);
        float z6 = z0;
        value += (a6 * a6) * (a6 * a6) * 
                 grad3(seed,
                      xrbp + (xNMask & PRIME_X),
                      yrbp + (~yNMask & PRIME_Y),
                      zrbp + (zNMask & PRIME_Z),
                      x6, y6, z6);
    }
    else {
        // Vertex 7: Displaced in X and Z directions (only tested if vertex 6 doesn't contribute)
        float a7 = xAFlipMask0 + zAFlipMask0 + a0;
        if (a7 > 0) {
            float x7 = x0 - (xNMask | 1);
            float y7 = y0;
            float z7 = z0 - (zNMask | 1);
            value += (a7 * a7) * (a7 * a7) * 
                     grad3(seed,
                          xrbp + (~xNMask & PRIME_X),
                          yrbp + (yNMask & PRIME_Y),
                          zrbp + (~zNMask & PRIME_Z),
                          x7, y7, z7);
        }
        
        // Vertex 8: Displaced in Y direction from second vertex
        float a8 = yAFlipMask1 + a1;
        if (a8 > 0) {
            float x8 = x1;
            float y8 = (yNMask | 1) + y1;
            float z8 = z1;
            value += (a8 * a8) * (a8 * a8) * 
                     grad3(seed2,
                          xrbp + PRIME_X,
                          yrbp + (yNMask & (PRIME_Y << 1)),
                          zrbp + PRIME_Z,
                          x8, y8, z8);
            skip9 = true; // Skip vertex 9 if vertex 8 contributes
        }
    }
    
    bool skipD = false;

    // Vertex A (10): Displaced in Z direction from first vertex
    float aA = zAFlipMask0 + a0;
    if (aA > 0) {
        float xA = x0;
        float yA = y0;
        float zA = z0 - (zNMask | 1);
        value += (aA * aA) * (aA * aA) * 
                 grad3(seed,
                      xrbp + (xNMask & PRIME_X),
                      yrbp + (yNMask & PRIME_Y),
                      zrbp + (~zNMask & PRIME_Z),
                      xA, yA, zA);
    }
    else {
        // Vertex B (11): Displaced in X and Y directions (only tested if vertex A doesn't contribute)
        float aB = xAFlipMask0 + yAFlipMask0 + a0;
        if (aB > 0) {
            float xB = x0 - (xNMask | 1);
            float yB = y0 - (yNMask | 1);
            float zB = z0;
            value += (aB * aB) * (aB * aB) * 
                     grad3(seed,
                          xrbp + (~xNMask & PRIME_X),
                          yrbp + (~yNMask & PRIME_Y),
                          zrbp + (zNMask & PRIME_Z),
                          xB, yB, zB);
        }
        
        // Vertex C (12): Displaced in Z direction from second vertex
        float aC = zAFlipMask1 + a1;
        if (aC > 0) {
            float xC = x1;
            float yC = y1;
            float zC = (zNMask | 1) + z1;
            value += (aC * aC) * (aC * aC) * 
                     grad3(seed2,
                          xrbp + PRIME_X,
                          yrbp + PRIME_Y,
                          zrbp + (zNMask & (PRIME_Z << 1)),
                          xC, yC, zC);
            skipD = true; // Skip vertex D if vertex C contributes
        }
    }
    
    // Conditional vertices, evaluated only if prior didn't contribute
    if (!skip5) {
        // Vertex 5: Displaced in Y and Z directions from second vertex
        float a5 = yAFlipMask1 + zAFlipMask1 + a1;
        if (a5 > 0) {
            float x5 = x1;
            float y5 = (yNMask | 1) + y1;
            float z5 = (zNMask | 1) + z1;
            value += (a5 * a5) * (a5 * a5) * 
                     grad3(seed2,
                          xrbp + PRIME_X,
                          yrbp + (yNMask & (PRIME_Y << 1)),
                          zrbp + (zNMask & (PRIME_Z << 1)),
                          x5, y5, z5);
        }
    }
    
    if (!skip9) {
        // Vertex 9: Displaced in X and Z directions from second vertex
        float a9 = xAFlipMask1 + zAFlipMask1 + a1;
        if (a9 > 0) {
            float x9 = (xNMask | 1) + x1;
            float y9 = y1;
            float z9 = (zNMask | 1) + z1;
            value += (a9 * a9) * (a9 * a9) * 
                     grad3(seed2,
                          xrbp + (xNMask & (PRIME_X * 2)),
                          yrbp + PRIME_Y,
                          zrbp + (zNMask & (PRIME_Z << 1)),
                          x9, y9, z9);
        }
    }
    
    if (!skipD) {
        // Vertex D (13): Displaced in X and Y directions from second vertex
        float aD = xAFlipMask1 + yAFlipMask1 + a1;
        if (aD > 0) {
            float xD = (xNMask | 1) + x1;
            float yD = (yNMask | 1) + y1;
            float zD = z1;
            value += (aD * aD) * (aD * aD) * 
                     grad3(seed2,
                          xrbp + (xNMask & (PRIME_X << 1)),
                          yrbp + (yNMask & (PRIME_Y << 1)),
                          zrbp + PRIME_Z,
                          xD, yD, zD);
        }
    }
    
    return value;
}

// Fractal 3D Simplex Noise
// An extension of the fractal noise algorithm to 3D simplex noise.
float Noise::fractalSimplexNoise3D(float x, float y, float z, int octaves, float persistence, float frequency, float lacunarity) const {
    float total = 0.0f;
    float freq = frequency;
    float amp = 1.0f;
    float maxAmp = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += simplexNoise3D(x * freq, y * freq, z * freq) * amp;
        maxAmp += amp;
        freq *= lacunarity;
        amp *= persistence;
    }

    float noise = total / maxAmp;
    return noise;
}