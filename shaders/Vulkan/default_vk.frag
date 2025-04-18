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
	vec4 ambientColor = vec4(sceneData.directionalLight.base.color, 1.0f) * sceneData.directionalLight.base.ambientIntensity;
	
	float diffuseFactor = max(dot(normalize(inNormal), normalize(sceneData.directionalLight.direction)), 0.0f);
	vec4 diffuseColor = vec4(sceneData.directionalLight.base.color, 1.0f) * sceneData.directionalLight.base.diffuseIntensity * diffuseFactor;
	
	vec4 specularColor = vec4(0,0,0,0);

	vec3 viewDir = normalize(inCameraPos - inPosition);
	vec3 reflectedVertex = normalize(reflect(-sceneData.directionalLight.direction, normalize(inNormal)));
	
	float specularFactor = dot(viewDir, reflectedVertex);
	if(specularFactor > 0.0f)
	{
		specularFactor = pow(specularFactor, sceneData.material.shininess);
		specularColor = vec4(sceneData.directionalLight.base.color * sceneData.material.specularIntensity * specularFactor, 1.0f);
	}

	outFragColor = vec4(texture(displayTexture, inUV).xyz, 1.0) * (ambientColor + diffuseColor + specularColor);
}