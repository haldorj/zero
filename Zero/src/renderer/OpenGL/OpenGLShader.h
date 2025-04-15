#pragma once

#include <glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

namespace Zero
{
	std::string ReadFile(const char* filename);

	class OpenGLShader
	{
	public:
		// Constructor that build the Shader Program from 2 different shaders
		OpenGLShader() = default;
		OpenGLShader(const char* vertexFile, const char* fragmentFile);

		// Activates the Shader Program
		void Activate() const;
		// Deletes the Shader Program
		void Delete() const;

		GLuint GetID() const { return ID; }

	private:
		static void CompileErrors(unsigned int shader, const char* type);

		// Reference ID of the Shader Program
		GLuint ID;
	};
}
