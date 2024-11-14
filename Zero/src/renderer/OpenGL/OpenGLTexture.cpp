#include "OpenGLTexture.h"


namespace Zero
{
	OpenGLTexture::OpenGLTexture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType)
	{
		type = texType;
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load(image, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels)
		{
			std::cout << "Failed to load texture file" << std::endl;
		}
		glGenTextures(1, &ID);
		glActiveTexture(slot);
		glBindTexture(GL_TEXTURE_2D, ID);

		glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLTexture::TexUnit(OpenGLShader& shader, const char* uniform, GLuint unit)
	{
		GLuint texUni = glGetUniformLocation(shader.GetID(), uniform);
		shader.Activate();
		glUniform1i(texUni, unit);
	}

	void OpenGLTexture::Bind()
	{
		glBindTexture(type, ID);
	}

	void OpenGLTexture::Unbind()
	{
		glBindTexture(type, 0);
	}

	void OpenGLTexture::Delete()
	{
		glDeleteTextures(1, &ID);
	}
} // namespace Zero