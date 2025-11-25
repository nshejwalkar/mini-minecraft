#version 330 core

uniform sampler2D u_Texture;

in vec2 fs_UV;
out vec4 out_Col;

void main() {
    vec4 color = texture(u_Texture, fs_UV);
    out_Col = mix(color, vec4(0.0, 0.0, 1.0, 1.0), 0.3);
}