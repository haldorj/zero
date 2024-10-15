#pragma once

#include "OpenGL/EBO.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/VAO.h"

#include "renderer/Renderer.h"

class OpenGLRenderer : public RendererBase
{
public:
	void Init() override;
	void InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices) override;
	void Shutdown() override;
	void SetClearColor(glm::vec4 clearColor) override { _clearColor = clearColor; }
	void Draw() override;

private:
	void InitShaders();

	glm::vec4 _clearColor = {};
	OpenGLShader* shaderProgram;
	VAO* VAO1;
	VBO* VBO1;
	EBO* EBO1;
};

