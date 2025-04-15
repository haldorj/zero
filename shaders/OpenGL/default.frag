#version 450

out vec4 FragColor;

in vec4 color;
in vec2 texCoord;
in vec3 normal;

struct DirectionalLight
{
	vec3 color;
	float ambientIntensity;
	vec3 direction;
	float diffuseIntensity;
};

uniform sampler2D tex0;
uniform DirectionalLight directionalLight;

void main()
{
	vec4 ambientColor = vec4(directionalLight.color, 1.0f) * directionalLight.ambientIntensity;

	float diffuseFactor = max(dot(normalize(normal), normalize(directionalLight.direction)), 0.0f);
	vec4 diffuseColor = vec4(directionalLight.color, 1.0f) * directionalLight.diffuseIntensity * diffuseFactor;

	FragColor = texture(tex0, texCoord) * (ambientColor + diffuseColor);
}