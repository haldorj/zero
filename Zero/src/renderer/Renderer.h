#pragma once

#include "glm/glm.hpp"
#include <shared/vk_types.h>

// Renderer interface
namespace Zero
{
	class RendererBase {
	public:
		virtual ~RendererBase() = default;

		virtual void Init() = 0;

		virtual void InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices) = 0;

		virtual void Shutdown() = 0;

		virtual void SetClearColor(glm::vec4 clearColor) = 0;

		virtual void Draw() = 0;
	};
}