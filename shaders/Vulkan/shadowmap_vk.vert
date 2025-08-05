#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require
#include "input_structures.glsl"

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
    Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	vec3 Pos = v.position;
    vec4 totalPosition = vec4(Pos, 1.0);

    if (animationData.animated == 1) // <-- ONLY if animated
    {
        totalPosition = vec4(0.0f);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (v.boneIds[i] == -1)
                continue;
            if (v.boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(Pos, 1.0f);
                break;
            }
            vec4 localPosition = animationData.finalBonesMatrices[v.boneIds[i]] * vec4(Pos, 1.0f);
            totalPosition += localPosition * v.weights[i];
        }
    }
    
    gl_Position = sceneData.directionalLightTransform * PushConstants.model * totalPosition;
}
