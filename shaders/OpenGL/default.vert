#version 450

// Positions/Coordinates
layout (location = 0) in vec3 Pos;
layout (location = 1) in float UV_x;
layout (location = 2) in vec3 Normal;
layout (location = 3) in float UV_y;
layout (location = 4) in vec4 Color;
layout (location = 5) in ivec4 boneIds; 
layout (location = 6) in vec4 weights;

out vec4 color;
out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;
out vec4 fragPosLight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightTransform;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform int Animated;

void main()
{
    vec4 totalPosition = vec4(Pos, 1.0f);

    if (Animated == 1) // <-- ONLY if animated
    {
        totalPosition = vec4(0.0f);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIds[i] == -1)
                continue;
            if (boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(Pos, 1.0f);
                break;
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(Pos, 1.0f);
            totalPosition += localPosition * weights[i];
        }
    }

    color = Color;
    texCoord = vec2(UV_x, UV_y);
    normal = mat3(transpose(inverse(model))) * Normal;

    fragPos = (model * totalPosition).xyz;
    fragPosLight = lightTransform * vec4(fragPos, 1.0f);

    gl_Position = projection * view * model * totalPosition;
}