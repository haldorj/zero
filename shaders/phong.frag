#version 450

// Outputs colors in RGBA
out vec4 FragColor;

in vec3 position;
in vec3 normal;
in vec2 texCoord;
in vec4 color;

// Gets the Texture Unit from the main function
uniform sampler2D tex0;

uniform vec3 viewPos;

void main()
{
	// Light pos
	vec3 lightPos = vec3(0, 12, 0);
	vec3 lightColor = vec3(0.8, 0.8, 0.8);

	// ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

	// diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - position);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - position);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;  

    vec3 result = (ambient + diffuse + specular) * texture(tex0, texCoord).xyz;

	FragColor = vec4(result, 1.0);
}