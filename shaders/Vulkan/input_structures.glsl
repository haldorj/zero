const int MAX_POINT_LIGHTS = 100;

#extension GL_EXT_scalar_block_layout : require

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

struct Material
{
	float specularIntensity;
    float shininess;
};

layout(scalar, set = 0, binding = 0) uniform SceneData 
{   
	mat4 view;
	mat4 proj;
	mat4 viewproj;

	// int pointLightCount;
	DirectionalLight directionalLight;
	Material material;
} sceneData;

//texture to access
layout(set = 0, binding = 1) uniform sampler2D displayTexture;