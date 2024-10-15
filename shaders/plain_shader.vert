#version 450

// shader inputs
layout (location = 0) in vec3 aPos;   // Vertex position
layout (location = 1) in vec3 aColor; // Vertex color

// output to fragment shader
layout (location = 0) out vec3 color;

uniform float scale;

void main()
{
   // Set position
   gl_Position = vec4(aPos.x * scale, aPos.y * scale, aPos.z * scale, 1.0);
   
   // Pass the color to the fragment shader
   color = aColor;
}
