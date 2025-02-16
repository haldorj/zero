#include "VkMesh.h"
#include <Renderer/VulkanRenderer.h>


namespace Zero {

    VkMesh::VkMesh(std::span<Vertex>& vertices, std::span<uint32_t>& indices, AllocatedImage texture, GPUMeshBuffers meshBuffers)
    {
        m_Vertices = vertices;
        m_Indices = indices;
        m_Texture = texture;
        m_GPUMeshBuffers = meshBuffers;
    }

}