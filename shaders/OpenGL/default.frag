#version 450

out vec4 FragColor;

in vec4 color;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec4 fragPosLight;

const int MAX_POINT_LIGHTS = 5;
const int MAX_SPOT_LIGHTS = 5;

struct Light
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct DirectionalLight
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};

struct SpotLight
{
	PointLight base;
	vec3 direction;
	float edge;
};

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform sampler2D tex0;
uniform sampler2D directionalShadowMap;
uniform Material material;

uniform vec3 viewPos;


float CalcDirectionalShadowFactor(DirectionalLight light)
{
	vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
	
	float shadow = 0.0f;
	if (projCoords.z <= 1.0f)
	{
		projCoords = (projCoords * 0.5) + 0.5; 

		float currentDepth = projCoords.z;
		float bias = max(0.005f * (1.0f - dot(normal, normalize(light.direction))), 0.0005f);

		int sampleRadius = 4;
		vec2 texelSize = 1.0 / textureSize(directionalShadowMap, 0);
		for (int y = -sampleRadius; y <= sampleRadius; y++)
		{
			for (int x = -sampleRadius; x <= sampleRadius; x++)
			{
				float pcfDepth = texture(directionalShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;  
			}
		}

		shadow /= pow((sampleRadius * 2 + 1), 2);
	}

	return shadow;
}

vec4 CalcLightByDirection(Light light, vec3 direction, float shadowFactor)
{
	vec4 ambientColor = vec4(light.color, 1.0f) * light.ambientIntensity;

	float diffuseFactor = max(dot(normalize(normal), normalize(direction)), 0.0f);
	vec4 diffuseColor = vec4(light.color, 1.0f) * light.diffuseIntensity * diffuseFactor;

	vec4 specularColor = vec4(0,0,0,0);

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = normalize(reflect(-direction, normalize(normal)));
	
	float specularFactor = dot(viewDir, reflectDir);
	if(specularFactor > 0.0f)
	{
		specularFactor = pow(specularFactor, material.shininess);
		specularColor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
	}

	return (ambientColor + (1.0f - shadowFactor) * (diffuseColor + specularColor));
}

vec4 CalcDirectionalLight()
{
	float shadowFactor = CalcDirectionalShadowFactor(directionalLight);
	return CalcLightByDirection(directionalLight.base, directionalLight.direction, shadowFactor);
}

vec4 CalcPointLight(PointLight pLight)
{
	vec3 direction = pLight.position - fragPos;
	float dist = length(direction);
	direction = normalize(direction);

	vec4 color = CalcLightByDirection(pLight.base, direction, 0.0f);
	float attenuation = (pLight.exponent * dist * dist	) +
						(pLight.linear 	 * dist			) +
						(pLight.constant				);

	return (color / attenuation);
}

vec4 CalcSpotLight(SpotLight sLight)
{
	vec3 direction = normalize(sLight.base.position - fragPos);
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
	for (int i = 0; i < pointLightCount; i++)
	{
		totalColor += CalcPointLight(pointLights[i]);
	}
	return totalColor;
}

vec4 CalcSpotLights()
{
	vec4 totalColor = vec4(0,0,0,0);
	for (int i = 0; i < spotLightCount; i++)
	{
		totalColor += CalcSpotLight(spotLights[i]);
	}
	return totalColor;
}

void main()
{
	vec4 finalColor = vec4(0,0,0,0);
	
	finalColor += CalcDirectionalLight();
	finalColor += CalcPointLights();
	finalColor += CalcSpotLights();

	FragColor = texture(tex0, texCoord) * finalColor;
}