#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout (location = 0) in vec2 TexCoords;

layout (location = 0) out vec4 FragColor;

void main()
{
    float depthValue = texture(displayTexture, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}
