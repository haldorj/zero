#version 450

out vec4 FragColor;

in vec4 color;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

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
uniform Material material;

uniform vec3 viewPos;

vec4 CalcLightByDirection(Light light, vec3 direction)
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

	return (ambientColor + diffuseColor + specularColor);
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection(directionalLight.base, directionalLight.direction);
}

vec4 CalcPointLight(PointLight pLight)
{
	vec3 direction = pLight.position - fragPos;
	float dist = length(direction);
	direction = normalize(direction);

	vec4 color = CalcLightByDirection(pLight.base, direction);
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