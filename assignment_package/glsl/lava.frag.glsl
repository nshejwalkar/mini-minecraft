#version 330 core

uniform sampler2D u_Texture;
uniform float u_Time;
uniform float u_TimeInLava;

in vec2 fs_UV;
out vec4 out_Col;

const float N = 100.0;

// taken from hw5
vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)),
                 dot(p, vec2(269.5,183.3))))
                 * 43758.5453);
}

float random1(ivec2 cell, float time) {
    vec3 k = vec3(float(cell.x), float(cell.y), time);
    return fract(sin(dot(k, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

// because random2 is pseudo-random, we can guarantee the same worley points in every shader call
// this returns the color of the closest worley point -> mosaic
vec2 WorleyNoise(vec2 uv) {
    uv *= N;  // Now the space is 10x10 instead of 1x1. Change this to any number you want.
    vec2 uvInt = floor(uv);
    vec2 uvFract = fract(uv);
    float minDist = 10.0;  // Minimum distance initialized to max.
    vec2 minPoint = vec2(-100, -100);  // init to anything

    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            vec2 neighbor = vec2(float(x), float(y));  // Direction in which neighbor cell lies
            vec2 point = random2(uvInt + neighbor);  // Get the Voronoi centerpoint for the neighboring cell
            point = 0.5 + 0.5 * sin(u_Time * 0.01 + 6 * point);  // dependent on time for movement
            vec2 diff = neighbor + point - uvFract;  // Distance between fragment coord and neighbor’s Voronoi point
            float dist = length(diff);
            if (dist < minDist) {
                minDist = dist;
                minPoint = (neighbor+point+uvInt) / N;
            }
        }
    }
    return minPoint;
}


void main()
{
    // screen turns woozy using Worley noise
    vec2 worleyPoint = WorleyNoise(fs_UV);
    vec4 texCol = texture(u_Texture, worleyPoint);

    // every second, turn random tiles red for a glitchy effect
    ivec2 cell = ivec2(floor(fs_UV*N));  // to get the current cell in N*N space
    float time = floor(u_Time/120);  // every second we change the sample

    float rand = random1(cell, time);

    // the longer you stay in lava, the more red your vision becomes!
    float expected = (100+15*u_TimeInLava)/(N*N);
    if (rand < expected) {
        out_Col = vec4(1,0,0,1);
    }
    else {
        out_Col = mix(texCol, vec4(1.0, 0.0, 0.0, 1.0), 0.35);
    }
}
