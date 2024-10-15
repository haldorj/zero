#include "OpenGLRenderer.h"
#include <core/core.h>
#include <core/Engine.h>
#include <iostream>

std::vector<GLfloat> rectVertices;
std::vector<GLuint> rectIndeces;

void OpenGLRenderer::Init()
{
	std::cout << "ZeroEngine OpenGL \n";

	glfwMakeContextCurrent(Engine::Get().GetWindow());

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << '\n';
		return;
	}

	glViewport(0, 0, EXTENT_WIDTH, EXTENT_HEIGHT);

	glfwSwapInterval(1); // vsync on

	const GLubyte* renderer = glGetString(GL_RENDERER);
	std::cout << "Chosen GPU: " << "\n";
	std::cout << "\t" << renderer << "\n";

	InitShaders();
}



void OpenGLRenderer::InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
	for (auto &vertex : vertices)
	{
		rectVertices.push_back((GLfloat)vertex.position.x); 
		rectVertices.push_back((GLfloat)vertex.position.y);
		rectVertices.push_back((GLfloat)vertex.position.z);
		rectVertices.push_back((GLfloat)vertex.color.r);
		rectVertices.push_back((GLfloat)vertex.color.g);
		rectVertices.push_back((GLfloat)vertex.color.b);
	}
	for (auto index : indices)
	{
		rectIndeces.push_back((GLuint)index);
	}
}

void OpenGLRenderer::Shutdown()
{
	// Delete all the objects we've created
	VAO1->Delete();
	VBO1->Delete();
	EBO1->Delete();
	shaderProgram->Delete();
}

void OpenGLRenderer::Draw()
{
	// Specify the color of the background
	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT);
	// Tell OpenGL which Shader Program we want to use
	shaderProgram->Activate();
	// Scale uniform. Must be called after the Shader Program has been activated.
	glUniform1f(uniID, 1.f);
	// Bind the VAO so OpenGL knows to use it
	VAO1->Bind();
	// Draw primitives, number of indices, datatype of indices, index of indices
	glDrawElements(GL_TRIANGLES, rectIndeces.size(), GL_UNSIGNED_INT, 0);
	// Swap the back buffer with the front buffer
	glfwSwapBuffers(Engine::Get().GetWindow());
}

void OpenGLRenderer::InitShaders()
{
	// Create Shader object
	shaderProgram = std::make_unique<OpenGLShader>("../shaders/plain_shader.vert", "../shaders/plain_shader.frag");

	// Generates Vertex Array Object and binds it
	VAO1 = std::make_unique<VAO>();
	VAO1->Bind();

	for (const auto vertex : rectVertices)
	{
		std::cout << vertex << "   ";
	}

	for (const auto index : rectIndeces)
	{
		std::cout << "\n" << index << "  ";
	};

	// Generates Vertex Buffer Object and links it to vertices
	VBO1 = std::make_unique<VBO>(rectVertices.data(), rectVertices.size() * sizeof(float));
	// Generates Element Buffer Object and links it to indices
	EBO1 = std::make_unique<EBO>(rectIndeces.data(), rectIndeces.size() * sizeof(GLuint));

	// Links VBO attributes such as coordinates and colors to VAO
	VAO1->LinkAttrib(*VBO1, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
	VAO1->LinkAttrib(*VBO1, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	// Unbind all to prevent accidentally modifying them
	VAO1->Unbind();
	VBO1->Unbind();
	EBO1->Unbind();

	uniID = glGetUniformLocation(shaderProgram->GetID(), "scale");
}

// Checks if the different Shaders have compiled properly
void OpenGLShader::compileErrors(unsigned int shader, const char* type)
{
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}