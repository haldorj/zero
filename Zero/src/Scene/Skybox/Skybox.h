#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <vector>
#include <string>
#include <Renderer/Vulkan/vk_types.h>
#include <Renderer/Vulkan/vk_descriptors.h>

namespace Zero {

	class Skybox
	{
	public:
		virtual ~Skybox() = default;

		virtual void LoadCubeMap(const std::vector<std::string>& faceLocations) {};

		virtual void Draw(const glm::mat4& projection, const glm::mat4& view) {};

		virtual void Draw(const VkCommandBuffer& cmd, const VkPipelineLayout& pipelineLayout,
			VkExtent2D drawExtent, GPUDrawPushConstants& pushConstants,
			DescriptorWriter& descriptorWriter) {}; 

		virtual void Destroy() const {};
	};

} 