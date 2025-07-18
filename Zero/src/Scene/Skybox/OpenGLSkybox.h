#pragma once
#include <string>

#include <Model/OpenGL/OpenGLMesh.h>
#include "Skybox.h"

namespace Zero {

	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox() = default;
		~OpenGLSkybox() = default;

		void Draw(const glm::mat4& projection, const glm::mat4& view) override;

		void LoadCubeMap(const std::vector<std::string>& faceLocations) override;
		void Destroy() const override {};
	private:
		OpenGLMesh* m_Mesh = nullptr;
		OpenGLShader* m_ShaderProgram = nullptr;

		GLuint m_TextureID = 0;
		GLuint m_UniformProjection = 0;
		GLuint m_UniformView = 0;

		GLuint skyboxVAO{}, skyboxVBO{}, skyboxEBO{};
	};

}
