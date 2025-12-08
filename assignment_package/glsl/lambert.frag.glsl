#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform sampler2D u_Texture;
uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform float u_Time;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec3 fs_ViewPos;
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec2 fs_UV;
flat in int fs_anim;
flat in int fs_overlay;
in vec3 fs_WorldPos;
in float fs_BiomeTemp;

out vec4 out_Col; // This is the final output color that you will see on your
// screen for the pixel that is currently being processed.

// Perlin noise functions
vec2 random2(vec2 p) {
    vec2 offset = p + vec2(42.0);
    return fract(sin(vec2(dot(offset, vec2(127.1, 311.7)),
                          dot(offset, vec2(269.5, 183.3))))
                 * 43758.5453);
}

float surflet(vec2 P, vec2 gridPoint) {
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);
    float tX = 1.0 - 6.0 * pow(distX, 5.0) + 15.0 * pow(distX, 4.0) - 10.0 * pow(distX, 3.0);
    float tY = 1.0 - 6.0 * pow(distY, 5.0) + 15.0 * pow(distY, 4.0) - 10.0 * pow(distY, 3.0);
    
    vec2 gradient = 2.0 * random2(gridPoint) - vec2(1.0);
    vec2 diff = P - gridPoint;
    
    float height = dot(diff, gradient);
    
    return height * tX * tY;
}

float perlinNoise(vec2 uv) {
    float surfletSum = 0.0;
    
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(uv, floor(uv) + vec2(float(dx), float(dy)));
        }
    }
    
    return surfletSum;
}

// Biome color functions
float getTemperatureNoise(vec2 pos) {
    const float TEMP_SCALE = 0.001;
    return perlinNoise((pos + vec2(10000.0, 10000.0)) * TEMP_SCALE);
}

vec3 interpolateBiomeColor(float temp, vec3 coldColor, vec3 warmColor, vec3 hotColor) {
    const float COLD_THRESHOLD = -0.2;
    const float HOT_THRESHOLD = 0.15;
    
    if (temp < COLD_THRESHOLD) {
        return coldColor;
    }
    else if (temp < HOT_THRESHOLD) {
        float t = (temp - COLD_THRESHOLD) / (HOT_THRESHOLD - COLD_THRESHOLD);
        return mix(coldColor, warmColor, t);
    }
    else {
        float t = clamp((temp - HOT_THRESHOLD) / 0.15, 0.0, 1.0);
        return mix(warmColor, hotColor, t);
    }
}

void main()
{
    vec2 uv = fs_UV;

    // Material base color (before shading)
    // vec4 diffuseColor = fs_Col;

    // ANIMATION
    if (fs_anim != 0) {
        float u64 = fs_UV.x * 64.0;
        float tile = floor(u64);
        float frac = fract(u64);
        float speed = 0.025;
        float newFrac = fract(frac + u_Time * speed);

        float animatedU64 = tile + newFrac;
        uv.x = animatedU64 / 64.0;
    }
    vec4 diffuseColor = texture(u_Texture, uv);
    
    // If overlay, apply biome tinting
    if (fs_overlay > 0) {
        // Get temperature
        float temp = fs_BiomeTemp;
        
        // Get tile offset
        vec2 tileOffset = fract(uv * vec2(64.0, 32.0));
        
        // Determine block color
        vec2 baseUV;
        vec3 biomeColor;
        
        // Grass top
        if (fs_overlay == 1) {
            vec3 snowyGrass = vec3(80.0, 102.0, 88.0) / 255.0;
            vec3 plainsGrass = vec3(78.0, 109.0, 59.0) / 255.0;
            vec3 desertGrass = vec3(109.0, 105.0, 57.0) / 255.0;
            biomeColor = interpolateBiomeColor(temp, snowyGrass, plainsGrass, desertGrass);
            baseUV = vec2(28.0, 32.0 - 10.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Grass side
        else if (fs_overlay == 2) {
            vec3 snowyGrass = vec3(80.0, 102.0, 88.0) / 255.0;
            vec3 plainsGrass = vec3(78.0, 109.0, 59.0) / 255.0;
            vec3 desertGrass = vec3(109.0, 105.0, 57.0) / 255.0;
            biomeColor = interpolateBiomeColor(temp, snowyGrass, plainsGrass, desertGrass);
            baseUV = vec2(28.0, 32.0 - 8.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Water
        else if (fs_overlay == 3) {
            vec3 snowyWater = vec3(40.0, 50.0, 110.0) / 255.0;
            vec3 plainsWater = vec3(45.0, 64.0, 118.0) / 255.0;
            vec3 desertWater = vec3(61.0, 110.0, 124.0) / 255.0;
            biomeColor = interpolateBiomeColor(temp, snowyWater, plainsWater, desertWater);
            baseUV = vec2(6.0, 32.0 - 2.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Oak leaves
        else if (fs_overlay == 4) {
            biomeColor = vec3(56.0, 82.0, 40.0) / 255.0;
            baseUV = vec2(21.0, 32.0 - 19.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Birch leaves
        else if (fs_overlay == 5) {
            biomeColor = vec3(56.0, 73.0, 62.0) / 255.0;
            baseUV = vec2(21.0, 32.0 - 19.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Cactus
        else if (fs_overlay == 6) {
            biomeColor = vec3(86.0, 140.0, 63.0) / 255.0;
            baseUV = vec2(25.0, 32.0 - 18.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Sample grayscale texture
        vec4 grayscaleColor = texture(u_Texture, baseUV + tileOffset / vec2(64.0, 32.0));
        
        // If grayscale value positive, tint the diffuse color
        if (grayscaleColor.r > 0.01) {
            diffuseColor.rgb = biomeColor * (grayscaleColor.r / 0.5);
        }
    }

    // LAMBERT SHADING
    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);
    float ambientTerm = 0.38;
    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.
    vec4 lambert_col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);

    // FOG
    float zdepth = -fs_ViewPos.z;
    float fogNear = 150.0;
    float fogFar = 500.0;
    float fogInterp = clamp((zdepth - fogNear) / (fogFar - fogNear), 0.0, 1.0);
    vec3 fogColor = vec3(0.75, 0.78, 0.80);
    vec3 finalRgb = mix(lambert_col.rgb, fogColor, fogInterp);

    // See through transparent blocks
    if (lambert_col.a < 0.5) {
        discard;
    }

    out_Col = vec4(finalRgb, lambert_col.a);
    // out_Col = vec4(vec3(zdepth / 100.0), 1.0);
    // out_Col = vec4(vec3(clamp(zdepth / 100.0, 0.0, 1.0)), 1.0);
}
