#include "OpenGLRenderer.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <core/core.h>
#include <core/Engine.h>

void OpenGLRenderer::Init()
{
	printf("ZeroEngine OpenGL \n");

	glfwMakeContextCurrent(Engine::Get().GetWindow());
	gladLoadGL();
	glViewport(0, 0, WIDTH, HEIGHT);

	glfwSwapInterval(1); // vsync on
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
