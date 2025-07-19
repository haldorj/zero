#version 450

layout (location  = 0) in vec3 aPos;

uniform mat4 lightTransform;
uniform mat4 model;

void main()
{
    gl_Position = lightTransform * model * vec4(aPos, 1.0f);
}