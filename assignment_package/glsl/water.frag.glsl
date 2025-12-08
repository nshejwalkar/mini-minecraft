#version 330 core

uniform sampler2D u_Texture;
uniform float u_Time;

in vec2 fs_UV;
out vec4 out_Col;

// decided not to use any noise functions, this looks cleaner
void main() {
    float wave1 = sin(fs_UV.y * 40.0 + u_Time * 1.5);
    float wave2 = cos(fs_UV.x * 30.0 + u_Time * 1.2);

    vec2 offset = vec2(wave1, wave2) * 0.002;
    vec2 uv = fs_UV + offset;

    vec4 color = texture(u_Texture, uv);

    out_Col = mix(color, vec4(0.0, 0.2, 0.8, 1.0), 0.35);
}
