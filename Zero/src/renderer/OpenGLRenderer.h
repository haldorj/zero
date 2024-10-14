#pragma once

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
	glm::vec4 _clearColor = {};
};

