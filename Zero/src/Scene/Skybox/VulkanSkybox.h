#pragma once

#include "Skybox.h"
#include <Model/Vertex.h>

namespace Zero {

	class VulkanSkybox final : public Skybox
	{
	public:
		VulkanSkybox() = default;
		~VulkanSkybox() override = default;

		VulkanSkybox(const std::vector<std::string>& faceLocations);
		
		void LoadCubeMap(const std::vector<std::string>& faceLocations) override;

		void Draw(const VkCommandBuffer& cmd, const VkPipelineLayout& pipelineLayout, 
		          VkExtent2D drawExtent, GPUDrawPushConstants& pushConstants,
		          DescriptorWriter& descriptorWriter) override;

		void Destroy() const override;
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
