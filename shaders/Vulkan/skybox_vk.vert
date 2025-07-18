#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

layout(buffer_reference, scalar) readonly buffer VertexBuffer
{ 
	Vertex vertices[];
};

layout(push_constant) uniform constants
{
	mat4 model;
	vec3 cameraPos;
	VertexBuffer vertexBuffer;
} PushConstants;

layout(location = 0) out vec3 outDirection;

void main()
{
    Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

    vec3 pos = v.position;
    mat4 view = mat4(mat3(cameraData.view));
    outDirection = pos;
    gl_Position = cameraData.proj * view * vec4(pos, 1.0);
}
