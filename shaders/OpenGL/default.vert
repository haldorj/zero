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

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    vec4 boneTransform = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            boneTransform = vec4(Pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(Pos,1.0f);
        boneTransform += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * Normal;
    }

	vec4 PosL = BoneTransform * vec4(Position, 1.0);

	// Outputs the positions/coordinates of all vertices
	gl_Position = projection * view * model * vec4(Pos, 1.0);
	// Assigns the colors from the Vertex Data to "color"
	color = Color;
	// Assigns the texture coordinates from the Vertex Data to "texCoord"
	texCoord = vec2(UV_x, UV_y);

	normal = mat3(transpose(inverse(model))) * Normal;

	fragPos = (model * vec4(Pos, 1.0)).xyz;
}