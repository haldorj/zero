#pragma once

#include <vector>
#include <string>
#include <Renderer/Vulkan/vk_types.h>
#include <Renderer/Vulkan/vk_descriptors.h>
#include <Model/Vertex.h>

namespace Zero {

	class VulkanSkybox
	{
	public:
		VulkanSkybox() = default;
		VulkanSkybox(std::vector<std::string> faceLocations);
		~VulkanSkybox() = default;

		void Draw(const VkCommandBuffer& cmd, const VkPipelineLayout& pipelineLayout, 
				  VkExtent2D drawExtent, GPUDrawPushConstants& pushConstants,
				  DescriptorWriter& descriptorWriter);

		void Destroy() const;
	private:
		AllocatedImage CreateCubeMap(const std::vector<std::string>& faceLocations);
		AllocatedImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped);

		void CreateSkyboxMesh();
		void CreateSampler();

		AllocatedImage m_Image{};
		VkSampler m_Sampler{};

		GPUMeshBuffers m_GPUMeshBuffers{};
		std::vector<uint32_t> m_Indices{};
		std::vector<Vertex> m_Vertices{};
	};

}
