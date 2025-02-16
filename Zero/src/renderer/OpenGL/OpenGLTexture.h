#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <renderer/OpenGL/OpenGLShader.h>

namespace Zero {

	class OpenGLTexture
	{
	public:
		OpenGLTexture() = default;
		OpenGLTexture(std::string path, std::string texType, GLuint slot, GLenum pixelType);

		void TexUnit(OpenGLShader& shader, std::string uniform, GLuint unit);
		void Bind();
		void Unbind();
		void Delete();

		std::string GetType() const { return TexType; }
		std::string GetFilePath() const { return Path; }

	private:
		GLuint ID;
		std::string TexType;
		GLuint Unit;

		std::string Path;
	};

} // namespace Zero
