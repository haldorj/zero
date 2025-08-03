#extension GL_EXT_scalar_block_layout : require

const int MAX_POINT_LIGHTS = 100;
const int MAX_SPOT_LIGHTS = 100;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

struct Vertex 
{
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;

	vec3 tangents;
	vec3 bitangents;

	ivec4 boneIds;
	vec4 weights;
}; 

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

layout(scalar, set = 0, binding = 0) readonly buffer SceneData 
{   
	int pointLightCount;
	int spotLightCount;
	
	DirectionalLight directionalLight;
	mat4 directionalLightTransform;
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
	Material material;
} sceneData;

layout(scalar, set = 0, binding = 1) uniform CameraData 
{   
	mat4 view;
	mat4 proj;
	mat4 viewproj;
} cameraData;

layout(set = 0, binding = 2) uniform samplerCube skyboxTexture;

layout(set = 0, binding = 3) uniform sampler2D directionalShadowMap;

layout(set = 1, binding = 0) uniform sampler2D displayTexture;

layout(scalar, set = 1, binding = 1) readonly buffer AnimationData
{
	int animated;
	mat4 finalBonesMatrices[MAX_BONES];
} animationData;