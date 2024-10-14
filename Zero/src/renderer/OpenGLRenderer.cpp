#include "OpenGLRenderer.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <core/core.h>
#include <core/Engine.h>
#include <iostream>

void OpenGLRenderer::Init()
{
    std::cout << "ZeroEngine OpenGL \n";

    glfwMakeContextCurrent(Engine::Get().GetWindow());

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glViewport(0, 0, EXTENT_WIDTH, EXTENT_HEIGHT);

    glfwSwapInterval(1); // vsync on

    const GLubyte* renderer = glGetString(GL_RENDERER);
    std::cout << "Chosen GPU: " << "\n";
    std::cout << "\t" << renderer << "\n";
}

void OpenGLRenderer::InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
}

void OpenGLRenderer::Shutdown()
{

}

void OpenGLRenderer::Draw()
{
	// Specify the color of the background
	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT);
	// Swap the back buffer with the front buffer
	glfwSwapBuffers(Engine::Get().GetWindow());
}
