#version 450
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D screenTexture;

void main()
{
    FragColor = texture(screenTexture, TexCoord);
}
