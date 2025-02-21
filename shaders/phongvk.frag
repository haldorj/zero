//glsl version 4.5
#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inPosition;

//texture to access
layout(set =0, binding = 0) uniform sampler2D displayTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	// Light pos
	vec3 lightPos = vec3(0, 12, 0);
	vec3 lightColor = vec3(1, 1, 1);

	// ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(lightPos - inPosition);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	vec3 result = (ambient + diffuse) * texture(displayTexture, inUV).xyz;

	outFragColor = vec4(result, 1.0);
}