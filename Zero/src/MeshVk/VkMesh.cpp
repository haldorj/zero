#include "VkMesh.h"
#include <Renderer/VulkanRenderer.h>
#include <Core/Application.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <Renderer/VulkanRenderer.h>

namespace Zero {

    VkMesh::VkMesh(std::span<Vertex>& vertices, std::span<uint32_t>& indices, AllocatedImage texture, GPUMeshBuffers meshBuffers)
    {
        m_Vertices = vertices;
        m_Indices = indices;
        m_Texture = texture;
        m_GPUMeshBuffers = meshBuffers;
    }

    void VkMesh::Draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout, VkExtent2D drawExtent, VkSampler sampler, GPUDrawPushConstants pushConstants)
    {
        auto renderer = static_cast<VulkanRenderer*>(Application::Get().GetRenderer());
        if (!renderer)
		{
			printf("VkMesh::Draw: Renderer is not of type VulkanRenderer");
			return;
		}

        //// TEXTURES /////////////////////////////////////////////////////////////////////////////////////////////////
        VkDescriptorSet imageSet = renderer->GetCurrentFrame().FrameDescriptors.Allocate(
            renderer->GetDevice(), renderer->GetSingleImageDescriptorLayout());
        {
            DescriptorWriter descriptorWriter;
            descriptorWriter.WriteImage(0, m_Texture.ImageView, sampler,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

            descriptorWriter.UpdateSet(renderer->GetDevice(), imageSet);
        }

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &imageSet, 0,
            nullptr);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////

        pushConstants.VertexBuffer = m_GPUMeshBuffers.VertexBufferAddress;

        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants),
            &pushConstants);
        vkCmdBindIndexBuffer(cmd, m_GPUMeshBuffers.IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, m_Indices.size(), 1, 0, 0, 0);
    }

}