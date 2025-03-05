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
	// Light pos
	vec3 lightPos = vec3(0, 12, 0);
	vec3 lightColor = vec3(0.8, 0.8, 0.8);

	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(lightPos - inPosition);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(inCameraPos - inPosition);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;  

	vec3 result = (ambient + diffuse + specular) * texture(displayTexture, inUV).xyz;

	outFragColor = vec4(result, 1.0);
}