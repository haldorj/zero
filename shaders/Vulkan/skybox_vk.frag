#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(skyboxTexture, inUV);
}