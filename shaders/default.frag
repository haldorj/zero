#version 450

// Outputs colors in RGBA
out vec4 FragColor;


// Inputs the color from the Vertex Shader
in vec3 color;
// Inputs the texture coordinates from the Vertex Shader
in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

// Gets the Texture Unit from the main function
uniform sampler2D tex0;
// Gets the color of the light from the main function
uniform vec4 lightColor;
// Gets the position of the light from the main function
uniform vec3 lightPos;

uniform vec3 camPos;

void main()
{
	float ambient = 0.2f;

	vec3 norm = normalize(normal);
	vec3 lightDirection = normalize(lightPos - fragPos);
	float diffuse = max(dot(norm, lightDirection), 0.0f);

	float specularStrength = 0.5f;
	vec3 viewDirection = normalize(camPos - fragPos);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 8.0f);
	float specular = specAmount * specularStrength;

	FragColor = texture(tex0, texCoord) * lightColor * (diffuse + ambient + specular);
}