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

layout(set = 0, binding = 0) uniform  SceneData {   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	DirectionalLight directionalLight;
	Material material;

} sceneData;

//texture to access
layout(set = 0, binding = 1) uniform sampler2D displayTexture;