#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 projection;

// Suporte a spritesheets e tiling (offset e escala de UV)
uniform vec2 uvOffset;
uniform vec2 uvScale;

void main()
{
    gl_Position = projection * model * vec4(position, 1.0f);
    TexCoord = uvOffset + tex_coord * uvScale;
}
