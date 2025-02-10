#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <renderer/OpenGL/OpenGLShader.h>

namespace Zero
{
	class OpenGLTexture
	{
	public:
		OpenGLTexture(const char* image, const char* texType, GLuint slot, GLenum format, GLenum pixelType);

		void TexUnit(OpenGLShader& shader, const char* uniform, GLuint unit);
		void Bind();
		void Unbind();
		void Delete();

		const char* GetType() const { return TexType; }

	private:
		GLuint ID;
		const char* TexType;
		GLuint Unit;
	};
} // namespace Zero
