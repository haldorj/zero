#version 450

layout(location = 0) in vec3 aPos;
layout (location = 1) in float UV_x;
layout (location = 2) in vec3 Normal;
layout (location = 3) in float UV_y;
layout (location = 4) in vec4 Color;
layout (location = 5) in ivec4 boneIds; 
layout (location = 6) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 model;
uniform mat4 lightTransform;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform int Animated;

void main()
{
    vec4 totalPosition = vec4(aPos, 1.0);

    if (Animated == 1)
    {
        totalPosition = vec4(0.0f);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIds[i] == -1)
                continue;
            if (boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(aPos, 1.0f);
                break;
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];
        }
    }
    else 
    {
        totalPosition = vec4(aPos, 1.0);
    }
    
    gl_Position = lightTransform * model * totalPosition;
}
