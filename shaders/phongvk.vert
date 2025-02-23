#version 450
#extension GL_EXT_buffer_reference : require

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outPosition;
layout (location = 4) out vec3 outCameraPos;

struct Vertex {

	// The reason the uv parameters are interleaved is is due to alignement limitations on GPUs. 
	// The GPU expects an alignment to 16 bytes by default (vec4 is smallest valid variable).

	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer{ 
	Vertex vertices[];
};

//push constants block
layout( push_constant ) uniform constants
{	
	mat4 model;
	mat4 viewproj;
	vec3 cameraPos;
	VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{	
	//load vertex data from device adress
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	//output data
	gl_Position = PushConstants.viewproj * PushConstants.model * vec4(v.position, 1.0f);

	outNormal = v.normal.xyz;
	outColor = v.color.xyz;
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;

	outCameraPos = PushConstants.cameraPos;

	outPosition = vec3(PushConstants.model * vec4(v.position, 1.0));
}
