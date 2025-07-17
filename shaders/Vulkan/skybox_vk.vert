#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout(buffer_reference, scalar) readonly buffer VertexBuffer
{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 model;
	vec3 cameraPos;
	VertexBuffer vertexBuffer;
} PushConstants;

layout (location = 0) out vec2 outUV;

void main() 
{
    Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	vec3 Pos = v.position;
    outUV.x = v.uv_x;
    outUV.y = v.uv_y;

    mat4 view = mat4(mat3(cameraData.view));

    gl_Position = cameraData.proj * view * vec4(Pos, 1.0);
}