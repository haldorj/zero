#extension GL_EXT_scalar_block_layout : require

const int MAX_POINT_LIGHTS = 100;
const int MAX_SPOT_LIGHTS = 100;

const int MAX_BONES = 100;

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

layout(scalar, set = 0, binding = 0) uniform SceneData 
{   
	mat4 view;
	mat4 proj;
	mat4 viewproj;

	int pointLightCount;
	int spotLightCount;
	
	DirectionalLight directionalLight;
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
	Material material;
} sceneData;

//texture to access
layout(set = 0, binding = 1) uniform sampler2D displayTexture;