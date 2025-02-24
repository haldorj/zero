#pragma once

#include <glm/glm.hpp>
#include <Renderer/OpenGL/OpenGLShader.h>
#include <vulkan/vulkan.h>
#include <Renderer/Vulkan/vk_types.h>

namespace Zero {

	class Model
	{
	public:
		virtual ~Model() = default;
		// OpenGL
		virtual void Draw(OpenGLShader& shader, glm::mat4& matrix) {};

		// Vulkan
		virtual void Draw(VkCommandBuffer& cmd, VkPipelineLayout& pipelineLayout, VkExtent2D drawExtent, VkSampler& sampler, GPUDrawPushConstants& pushConstants) {};
		virtual void DestroyModel() const {};
	};

}
