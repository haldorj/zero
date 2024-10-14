#pragma once

#include "renderer/Renderer.h"
#include <glad/glad.h>

class OpenGLRenderer : public RendererBase
{
public:
	void Init() override;

	void InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices) override;

	void Shutdown() override;

	void SetClearColor(glm::vec4 clearColor) override { _clearColor = clearColor; }

	void Draw() override;

private:
	glm::vec4 _clearColor = {};
	GLuint shaderProgram;

	// Create reference containers for the Vartex Array Object and the Vertex Buffer Object
	GLuint VAO, VBO, EBO;
};

