#pragma once
#include <string>

#include <Model/OpenGL/OpenGLMesh.h>
#include <vector>

namespace Zero {

	class Skybox
	{
	public:
		Skybox() = default;
		Skybox(std::vector<std::string> faceLocations);
		~Skybox() = default;

		void Draw(const glm::mat4& projection, const glm::mat4& view);

	private:
		OpenGLMesh* m_Mesh = nullptr;
		OpenGLShader* m_ShaderProgram = nullptr;

		GLuint m_TextureID = 0;
		GLuint m_UniformProjection = 0;
		GLuint m_UniformView = 0;

		GLuint skyboxVAO{}, skyboxVBO{}, skyboxEBO{};
	};

}
