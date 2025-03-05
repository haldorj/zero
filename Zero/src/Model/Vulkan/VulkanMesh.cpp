#include "VulkanMesh.h"
#include <Renderer/VulkanRenderer.h>
#include <Application.h>
#include <Renderer/Vulkan/VulkanBuffer.h>

namespace Zero
{
    VulkanMesh::VulkanMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                           const std::vector<VulkanTexture>& textures)
    {
        const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        m_Vertices = vertices;
        m_Indices = indices;
        m_Textures = textures;

        if (m_Textures.empty())
        {
            m_Textures.emplace_back(renderer->GetDefaultTexture());
        }

        m_GPUMeshBuffers = renderer->UploadMesh(indices, vertices);
    }

    void VulkanMesh::Draw(const VkCommandBuffer& cmd, DescriptorWriter& descriptorWriter, const VkPipelineLayout& pipelineLayout, VkExtent2D drawExtent,
                          const VkSampler& sampler, GPUDrawPushConstants& pushConstants) const
    {
        const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
        if (!renderer)
        {
            printf("VkMesh::Draw: Renderer is not of type VulkanRenderer");
            return;
        }

        //// TEXTURES /////////////////////////////////////////////////////////////////////////////////////////////////

        const VkDescriptorSet imageSet = renderer->GetCurrentFrame().FrameDescriptors.Allocate(
            renderer->GetDevice(), renderer->GetSingleImageDescriptorLayout());
        {
            for (auto& texture : m_Textures)
            {
                // DescriptorWriter descriptorWriter;
                descriptorWriter.WriteImage(1, texture.GetImage().ImageView, sampler,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

                descriptorWriter.UpdateSet(renderer->GetDevice(), imageSet);
            }
        }

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &imageSet, 0,
                                nullptr);

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        pushConstants.VertexBuffer = m_GPUMeshBuffers.VertexBufferAddress;

        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants),
                           &pushConstants);
        vkCmdBindIndexBuffer(cmd, m_GPUMeshBuffers.IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
    }

    void VulkanMesh::DestroyMesh() const
    {
        const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
        vkDeviceWaitIdle(renderer->GetDevice());
        
        VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), m_GPUMeshBuffers.IndexBuffer);
        VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), m_GPUMeshBuffers.VertexBuffer);
    }
}
