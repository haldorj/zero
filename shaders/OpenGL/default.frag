#version 450

out vec4 FragColor;

in vec4 color;
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

struct DirectionalLight
{
	vec3 color;
	float ambientIntensity;
	vec3 direction;
	float diffuseIntensity;
};

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform sampler2D tex0;
uniform DirectionalLight directionalLight;
uniform Material material;

uniform vec3 viewPos;

void main()
{
	vec4 ambientColor = vec4(directionalLight.color, 1.0f) * directionalLight.ambientIntensity;

	float diffuseFactor = max(dot(normalize(normal), normalize(directionalLight.direction)), 0.0f);
	vec4 diffuseColor = vec4(directionalLight.color, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;

	vec4 specularColor = vec4(0,0,0,0);
	if (diffuseFactor > 0.0f)
	{
		vec3 fragToView = normalize(viewPos - fragPos);
		vec3 reflectedVertex = normalize(reflect(directionalLight.direction, normalize(normal)));
		
		float specularFactor = dot(-fragToView, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularColor = vec4(directionalLight.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}

	FragColor = texture(tex0, texCoord) * (ambientColor + diffuseColor + specularColor);
}