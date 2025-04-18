#version 450

out vec4 FragColor;

in vec4 color;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

const int MAX_POINT_LIGHTS = 100;

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

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform int pointLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

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

vec4 CalcPointLights()
{
	vec4 totalColor = vec4(0,0,0,0);
	for (int i = 0; i < pointLightCount; i++)
	{
		vec3 direction = pointLights[i].position - fragPos;
		float dist = length(direction);
		direction = normalize(direction);

		vec4 color = CalcLightByDirection(pointLights[i].base, direction);
		float attenuation = (pointLights[i].exponent * dist * dist	) +
							(pointLights[i].linear 	 * dist			) +
							(pointLights[i].constant				);

		totalColor += (color / attenuation);
	}
	return totalColor;
}

void main()
{
	vec4 finalColor = CalcDirectionalLight();
	finalColor += CalcPointLights();

	FragColor = texture(tex0, texCoord) * finalColor;
}