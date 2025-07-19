#pragma once

#include <glad/glad.h>
#include <Renderer/OpenGL/OpenGLShader.h>
#include <glm/glm.hpp>
#include <Scene/Scene.h>

namespace Zero {

	class OpenGLShadowmap
	{
	public:
		OpenGLShadowmap();
		~OpenGLShadowmap();
		void Init();

		void Draw(Scene* scene);

		OpenGLShader* GetShaderProgram() const { return m_ShaderProgram; }
		GLuint GetDepthMapTexture() const { return m_DepthMapTexture; }

	private:
		glm::mat4 projectionMatrix{};

		OpenGLShader* m_ShaderProgram{};

		GLuint m_DepthMapFBO{};
		GLuint m_DepthMapTexture;
	};
}


