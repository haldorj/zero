#version 450

// Input from vertex shader
layout (location = 0) in vec3 inColor;

// Output to the framebuffer
layout (location = 0) out vec4 outFragColor;

void main()
{
   // Output the color with an alpha value of 1.0 (opaque)
   outFragColor = vec4(inColor, 1.0f);
}