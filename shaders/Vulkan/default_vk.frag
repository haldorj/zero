// glsl version 4.5
#version 450

#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inPosition;
layout (location = 4) in vec3 inCameraPos;
layout (location = 5) in vec4 inFragPosLight;

layout (location = 0) out vec4 outFragColor;

float CalcDirectionalShadowFactor(DirectionalLight light)
{
    vec4 lightSpacePos = inFragPosLight; // already lightMatrix * worldPos
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;

    // Only x and y need to be remapped from [-1, 1] to [0, 1]
    vec2 texCoords = projCoords.xy * 0.5 + 0.5;
    float currentDepth = projCoords.z; // Already in [0, 1] range if Vulkan projection fix was applied

	float shadow = 0.0;

    if (currentDepth > 1.0 || 
        texCoords.x < 0.0 || texCoords.x > 1.0 || 
        texCoords.y < 0.0 || texCoords.y > 1.0)
    {
        return shadow;
    }

	if (projCoords.z <= 1.0f)
	{
		float bias = max(0.01f * (1.0f - dot(inNormal, normalize(light.direction))), 0.0005f);

		int sampleRadius = 2;
		vec2 texelSize = 1.0 / textureSize(directionalShadowMap, 0);
		for(int x = -sampleRadius; x <= sampleRadius; ++x)
		{
			for(int y = -sampleRadius; y <= sampleRadius; ++y)
			{
				float pcfDepth = texture(directionalShadowMap, texCoords.xy + vec2(x, y) * texelSize).r;
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
			}
		};
		shadow /= pow((sampleRadius * 2 + 1), 2);
	}

    return shadow;
}


vec4 CalcLightByDirection(Light light, vec3 direction, float shadowFactor)
{
	vec4 ambientColor = vec4(light.color, 1.0f) * light.ambientIntensity;

	float diffuseFactor = max(dot(normalize(inNormal), normalize(direction)), 0.0f);
	vec4 diffuseColor = vec4(light.color, 1.0f) * light.diffuseIntensity * diffuseFactor;

	vec4 specularColor = vec4(0,0,0,0);

	vec3 viewDir = normalize(inCameraPos - inPosition);
	vec3 reflectDir = normalize(reflect(-direction, normalize(inNormal)));
	
	float specularFactor = dot(viewDir, reflectDir);
	if(specularFactor > 0.0f)
	{
		specularFactor = pow(specularFactor, sceneData.material.shininess);
		specularColor = vec4(light.color * sceneData.material.specularIntensity * specularFactor, 1.0f);
	}

	return (ambientColor + (1.0f - shadowFactor) * (diffuseColor + specularColor));
}

vec4 CalcDirectionalLight()
{
	float shadowFactor = CalcDirectionalShadowFactor(sceneData.directionalLight);
	return CalcLightByDirection(sceneData.directionalLight.base, sceneData.directionalLight.direction, shadowFactor);
}

vec4 CalcPointLight(PointLight pLight)
{
	float shadowFactor = 0.0f;

	vec3 direction = pLight.position - inPosition;
	float dist = length(direction);
	direction = normalize(direction);

	vec4 color = CalcLightByDirection(pLight.base, direction, shadowFactor);
	float attenuation = (pLight.exponent * dist * dist	) +
						(pLight.linear 	 * dist			) +
						(pLight.constant				);

	return (color / attenuation);
}

vec4 CalcSpotLight(SpotLight sLight)
{
	vec3 direction = normalize(sLight.base.position - inPosition);
	float slFactor = dot(direction, normalize(-sLight.direction));
	if (slFactor > sLight.edge)
	{
		vec4 color = CalcPointLight(sLight.base);

		return color * (1.0f - (1.0f - slFactor) * (1.0f/(1.0f - sLight.edge)));
	} 
	else
	{
		return vec4(0,0,0,0);
	}
}


vec4 CalcPointLights()
{
	vec4 totalColor = vec4(0,0,0,0);
	for (int i = 0; i < sceneData.pointLightCount; i++)
	{
		totalColor += CalcPointLight(sceneData.pointLights[i]);
	}
	return totalColor;
}

vec4 CalcSpotLights()
{
	vec4 totalColor = vec4(0,0,0,0);
	for (int i = 0; i < sceneData.spotLightCount; i++)
	{
		totalColor += CalcSpotLight(sceneData.spotLights[i]);
	}
	return totalColor;
}

void main() 
{
	vec4 finalColor = CalcDirectionalLight();
	finalColor += CalcPointLights();
	finalColor += CalcSpotLights();

	outFragColor = vec4(texture(displayTexture, inUV).xyz, 1.0) * finalColor;
}