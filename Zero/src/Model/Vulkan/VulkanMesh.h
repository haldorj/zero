#pragma once
#include <vector>
#include <Model/Vertex.h>
#include <span>

#include <Renderer/Vulkan/vk_types.h>
#include <Renderer/Vulkan/VulkanTexture.h>

class VulkanRenderer;

namespace Zero {

	class VulkanMesh
	{
	public:
		VulkanMesh() = default;
		VulkanMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, const std::vector<VulkanTexture>& textures);

		void Draw(const VkCommandBuffer& cmd,
		          const VkPipelineLayout& pipelineLayout,
		          VkExtent2D drawExtent,
		          const VkSampler& sampler,
		          GPUDrawPushConstants& pushConstants
		) const;

		void DestroyMesh() const;

		std::vector<Vertex> GetVertices() const { return m_Vertices; }
		std::vector<uint32_t> GetIndices() const { return m_Indices; }
		std::vector<VulkanTexture> GetTexture() const { return m_Textures; }
		GPUMeshBuffers GetGPUMeshBuffers() const { return m_GPUMeshBuffers; }

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<VulkanTexture> m_Textures;
		GPUMeshBuffers m_GPUMeshBuffers;
	};

}
