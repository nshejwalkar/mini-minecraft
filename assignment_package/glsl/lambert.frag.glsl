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

out vec4 out_Col; // This is the final output color that you will see on your
// screen for the pixel that is currently being processed.

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
        // Get tile offset
        vec2 tileOffset = fract(uv * vec2(64.0, 32.0));
        
        // Determine block color
        vec2 baseUV;
        vec3 biomeColor;
        
        // Grass top
        if (fs_overlay == 1) {
            biomeColor = vec3(78.0, 109.0, 59.0) / 255.0;
            baseUV = vec2(28.0, 32.0 - 10.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Grass side
        else if (fs_overlay == 2) {
            biomeColor = vec3(78.0, 109.0, 59.0) / 255.0;
            baseUV = vec2(28.0, 32.0 - 8.0 - 1.0) / vec2(64.0, 32.0);
        }
        
        // Water
        else if (fs_overlay == 3) {
            biomeColor = vec3(45.0, 65.0, 119.0) / 255.0;
            baseUV = vec2(6.0, 32.0 - 2.0 - 1.0) / vec2(64.0, 32.0);
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
    float ambientTerm = 0.35;
    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.
    vec4 lambert_col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);

    // FOG
    float zdepth = -fs_ViewPos.z;
    float fogNear = 200.0;
    float fogFar = 1000.0;
    float fogInterp = clamp((zdepth - fogNear) / (fogFar - fogNear), 0.0, 1.0);
    vec3 fogColor = vec3(0.75, 0.78, 0.80);
    vec3 finalRgb = mix(lambert_col.rgb, fogColor, fogInterp);

    out_Col = vec4(finalRgb, lambert_col.a);
    // out_Col = vec4(vec3(zdepth / 100.0), 1.0);
    // out_Col = vec4(vec3(clamp(zdepth / 100.0, 0.0, 1.0)), 1.0);
}
