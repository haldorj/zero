#version 450

// Positions/Coordinates
layout (location = 0) in vec3 Pos;
layout (location = 1) in float UV_x;
layout (location = 2) in vec3 Normal;
layout (location = 3) in float UV_y;
layout (location = 4) in vec4 Color;

out vec3 position;
out vec3 normal;
out vec2 texCoord;
out vec4 color;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;

void main()
{
	gl_Position = projection * view * model * vec4(Pos, 1.0);

	position = vec3(model * vec4(Pos, 1.0));
	normal = mat3(transpose(inverse(model))) * Normal;
	texCoord = vec2(UV_x, UV_y);
	color = Color;
	
}