
layout(set = 0, binding = 0) uniform  SceneData{   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;

} sceneData;

//texture to access
layout(set = 0, binding = 1) uniform sampler2D displayTexture;