#include "VulkanMesh.h"
#include <Renderer/VulkanRenderer.h>
#include <Application.h>
#include <Renderer/Vulkan/VulkanBuffer.h>
#include <vk_mem_alloc.h>

namespace Zero
{
    VulkanMesh::VulkanMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                           const std::vector<VulkanTexture>& textures)
    {
        const auto& renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        m_Vertices = vertices;
        m_Indices = indices;
        m_Textures = textures;

        if (m_Textures.empty())
        {
            m_Textures.emplace_back(renderer->GetDefaultTexture());
        }

        m_GPUMeshBuffers = renderer->UploadMesh(indices, vertices);
    }

    void VulkanMesh::Draw(const VkCommandBuffer& cmd, const VkPipelineLayout& pipelineLayout,
                          const VkSampler& sampler, GPUDrawPushConstants& pushConstants, DescriptorWriter& descriptorWriter)
    {
        const auto& renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
        if (!renderer)
        {
            printf("VkMesh::Draw: Renderer is not of type VulkanRenderer");
            return;
        }

        //// TEXTURES /////////////////////////////////////////////////////////////////////////////////////////////////

        const VkDescriptorSet descriptorSet = renderer->GetCurrentFrame().FrameDescriptors.Allocate(
            renderer->GetDevice(), renderer->GetGameObjectDescriptorLayout());

        for (auto& texture : m_Textures)
        {
            // DescriptorWriter descriptorWriter;
            descriptorWriter.WriteImage(0, texture.GetImage().ImageView, sampler,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

            descriptorWriter.UpdateSet(renderer->GetDevice(), descriptorSet);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        descriptorWriter.UpdateSet(renderer->GetDevice(), descriptorSet);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &descriptorSet, 0, nullptr);

        pushConstants.VertexBuffer = m_GPUMeshBuffers.VertexBufferAddress;

        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants),
                           &pushConstants);
        vkCmdBindIndexBuffer(cmd, m_GPUMeshBuffers.IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
    }

    void VulkanMesh::DestroyMesh() const
    {
        const auto& renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
        vkDeviceWaitIdle(renderer->GetDevice());
        
        VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), m_GPUMeshBuffers.IndexBuffer);
        VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), m_GPUMeshBuffers.VertexBuffer);
    }
}
