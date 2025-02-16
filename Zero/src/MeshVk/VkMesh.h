#pragma once
#include <vector>
#include <Mesh/Vertex.h>
#include <span>
#include <Renderer/Vulkan/vk_types.h>

namespace Zero {

	class VkMesh
	{
	public:
		VkMesh() = default;
		VkMesh(std::span <Vertex>& vertices, std::span <uint32_t>& indices, AllocatedImage texture, GPUMeshBuffers meshBuffers);

		std::span <Vertex> GetVertices() const { return m_Vertices; }
		std::span <uint32_t> GetIndices() const { return m_Indices; }
		AllocatedImage GetTexture() const { return m_Texture; }
		GPUMeshBuffers GetGPUMeshBuffers() const { return m_GPUMeshBuffers; }

	private:
		std::span <Vertex> m_Vertices;
		std::span <uint32_t> m_Indices;
		AllocatedImage m_Texture;
		GPUMeshBuffers m_GPUMeshBuffers;
	};

}
