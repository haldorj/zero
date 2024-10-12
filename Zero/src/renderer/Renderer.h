#pragma once

#include "glm/glm.hpp"

// Renderer interface
class Renderer {
public:
	virtual ~Renderer() = default;

	virtual void Init() = 0;

	virtual void Shutdown() = 0;

	virtual void SetClearColor(glm::vec4 clearColor) = 0;

	virtual void Draw() = 0;
};
