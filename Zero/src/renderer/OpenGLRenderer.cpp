#include "OpenGLRenderer.h"
#include <core/core.h>
#include <core/Engine.h>
#include <iostream>

// Vertices coordinates
GLfloat vertices[] =
{
	-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower left corner
	0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower right corner
	0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f, // Upper corner
	-0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Inner left
	0.5f / 2, 0.5f * float(sqrt(3)) / 6, 0.0f, // Inner right
	0.0f, -0.5f * float(sqrt(3)) / 3, 0.0f // Inner down
};

// Indices for vertices order
GLuint indices[] =
{
	0, 3, 5, // Lower left triangle
	3, 2, 4, // Upper triangle
	5, 4, 1 // Lower right triangle
};

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
	// Bind the VAO so OpenGL knows to use it
	VAO1->Bind();
	// Draw primitives, number of indices, datatype of indices, index of indices
	glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
	// Swap the back buffer with the front buffer
	glfwSwapBuffers(Engine::Get().GetWindow());
}

void OpenGLRenderer::InitShaders()
{
	// Create Shader object
	shaderProgram = new OpenGLShader("../shaders/plain_shader.vert", "../shaders/plain_shader.frag");

	// Generates Vertex Array Object and binds it
	VAO1 = new VAO();
	VAO1->Bind();

	// Generates Vertex Buffer Object and links it to vertices
	VBO1 = new VBO(vertices, sizeof(vertices));
	// Generates Element Buffer Object and links it to indices
	EBO1 = new EBO(indices, sizeof(indices));

	// Links VBO to VAO
	VAO1->LinkVBO(*VBO1, 0);
	// Unbind all to prevent accidentally modifying them
	VAO1->Unbind();
	VBO1->Unbind();
	EBO1->Unbind();
}
