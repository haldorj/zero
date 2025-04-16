#version 450

// Positions/Coordinates
layout (location = 0) in vec3 Pos;
layout (location = 1) in float UV_x;
layout (location = 2) in vec3 Normal;
layout (location = 3) in float UV_y;
layout (location = 4) in vec4 Color;

out vec4 color;
out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	// Outputs the positions/coordinates of all vertices
	gl_Position = projection * view * model * vec4(Pos, 1.0);
	// Assigns the colors from the Vertex Data to "color"
	color = Color;
	// Assigns the texture coordinates from the Vertex Data to "texCoord"
	texCoord = vec2(UV_x, UV_y);

	normal = mat3(transpose(inverse(model))) * Normal;

	fragPos = (model * vec4(Pos, 1.0)).xyz;
}