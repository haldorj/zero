#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <renderer/OpenGL/OpenGLShader.h>

namespace Zero {

	class OpenGLTexture
	{
	public:
		OpenGLTexture() = default;
		OpenGLTexture(const char* path, const char* texType, GLuint slot, GLenum pixelType);

		void TexUnit(OpenGLShader& shader, const char* uniform, GLuint unit);
		void Bind();
		void Unbind();
		void Delete();

		const char* GetType() const { return TexType; }
		const char* GetPath() const { return Path; }

	private:
		GLuint ID;
		const char* TexType;
		GLuint Unit;

		const char* Path;
	};

} // namespace Zero
