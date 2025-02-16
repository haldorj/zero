#pragma once
#include <vector>
#include <Mesh/Vertex.h>
#include <span>

#include <Renderer/Vulkan/vk_types.h>

class VulkanRenderer;

namespace Zero {

	class VkMesh
	{
	public:
		VkMesh() = default;
		VkMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<AllocatedImage> textures);

		void Draw(VkCommandBuffer cmd, 
			VkPipelineLayout pipelineLayout, 
			VkExtent2D drawExtent, 
			VkSampler sampler,
			GPUDrawPushConstants pushConstants
		);

		std::vector<Vertex> GetVertices() const { return m_Vertices; }
		std::vector<uint32_t> GetIndices() const { return m_Indices; }
		std::vector<AllocatedImage> GetTexture() const { return m_Textures; }
		GPUMeshBuffers GetGPUMeshBuffers() const { return m_GPUMeshBuffers; }

	private:
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<AllocatedImage> m_Textures;
		GPUMeshBuffers m_GPUMeshBuffers;
	};

}
