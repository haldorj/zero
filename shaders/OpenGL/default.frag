#version 450

out vec4 FragColor;

in vec4 color;
in vec2 texCoord;

struct DirectionalLight
{
	vec3 color;
	float ambientIntensity;
};

uniform sampler2D tex0;
uniform DirectionalLight directionalLight;

void main()
{
	vec4 ambientColor = vec4(directionalLight.color, 1.0f) * directionalLight.ambientIntensity;

	FragColor = texture(tex0, texCoord) * ambientColor;
}