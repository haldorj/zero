#pragma once

#include <glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

namespace Zero
{
	std::string GetFileContents(const char* filename);

	class OpenGLShader
	{
	public:
		// Constructor that build the Shader Program from 2 different shaders
		OpenGLShader() = default;
		OpenGLShader(const char* vertexFile, const char* fragmentFile);

		// Activates the Shader Program
		void Activate();
		// Deletes the Shader Program
		void Delete();

		GLuint GetID() const { return ID; }

	private:
		void compileErrors(unsigned int shader, const char* Type);

		// Reference ID of the Shader Program
		GLuint ID;
	};
}
