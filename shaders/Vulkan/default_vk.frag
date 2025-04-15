// glsl version 4.5
#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inPosition;
layout (location = 4) in vec3 inCameraPos;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec4 ambientColor = vec4(sceneData.directionalLight.color, 1.0f) * sceneData.directionalLight.ambientIntensity;
	outFragColor = vec4(texture(displayTexture, inUV).xyz, 1.0) * ambientColor;
}