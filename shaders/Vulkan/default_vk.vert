// glsl version 4.5
#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outPosition;
layout (location = 4) out vec3 outCameraPos;

struct Vertex {

	// The reason the uv parameters are interleaved is is due to alignement limitations on GPUs. 
	// The GPU expects an alignment to 16 bytes by default (vec4 is smallest valid variable).

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

layout(buffer_reference, scalar) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 model;
	vec3 cameraPos;
	// int Animated;
   	// mat4 BoneMatrices[MAX_BONES];
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	//load vertex data from device adress
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = sceneData.viewproj * PushConstants.model * vec4(v.position, 1.0f);

	outNormal = mat3(transpose(inverse(PushConstants.model))) * v.normal.xyz;
	outColor = v.color.xyz;
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;

	outCameraPos = PushConstants.cameraPos;

	outPosition = vec3(PushConstants.model * vec4(v.position, 1.0));
}
