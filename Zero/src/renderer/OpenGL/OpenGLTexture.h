#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <renderer/OpenGL/OpenGLShader.h>

namespace Zero
{
	class OpenGLTexture
	{
	public:
		OpenGLTexture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType);

		// Assigns a texture unit to a texture
		void texUnit(OpenGLShader& shader, const char* uniform, GLuint unit);
		// Binds a texture
		void Bind();
		// Unbinds a texture
		void Unbind();
		// Deletes a texture
		void Delete();
	private:
		GLuint ID;
		GLenum type;
	};
} // namespace Zero
