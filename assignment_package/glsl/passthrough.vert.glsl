#version 330 core

in vec4 vs_Pos;

out vec2 fs_UV;

void main()
{
    fs_UV = (vs_Pos.xy + 1) / 2;
    gl_Position = vs_Pos;
}