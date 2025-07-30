#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout (location = 0) out vec2 TexCoord;

layout(buffer_reference, scalar) readonly buffer VertexBuffer
{ 
	Vertex vertices[];
};

layout( push_constant ) uniform constants
{	
	mat4 model;
	vec3 cameraPos;
	VertexBuffer vertexBuffer;
} PushConstants;

void main()
{
    Vertex v =  PushConstants.vertexBuffer.vertices[gl_VertexIndex];
    TexCoord = vec2(v.uv_x, v.uv_y);
    gl_Position = vec4(v.position.xy, 0.0, 1.0);
}