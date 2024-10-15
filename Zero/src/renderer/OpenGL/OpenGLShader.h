#pragma once

#include <glad/glad.h>
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

std::string get_file_contents(const char* filename);

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

private:
	// Reference ID of the Shader Program
	GLuint ID;
};
