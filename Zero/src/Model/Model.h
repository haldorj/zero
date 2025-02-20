#pragma once

#include <glm/glm.hpp>
#include <Renderer/OpenGL/OpenGLShader.h>
#include <vulkan/vulkan.h>
#include <Renderer/Vulkan/vk_types.h>

namespace Zero {

	class Model
	{
	public:
		// OpenGL
		virtual void Draw(OpenGLShader& shader) {};

		// Vulkan
		virtual void Draw(VkCommandBuffer& cmd, VkPipelineLayout& pipelineLayout, VkExtent2D drawExtent, VkSampler& sampler, GPUDrawPushConstants& pushConstants) {};

		virtual void DestroyModel() const {};

		glm::mat4 GetMatrix() const { return m_Matrix; }
		void SetMatrix(const glm::mat4& matrix) { m_Matrix = matrix; }

	protected:
		glm::mat4 m_Matrix{ 1.0f };
	};

}
