#include "VulkanShadowmap.h"

#include "Application.h"
#include "core/core.h"
#include "Renderer/Vulkan/vk_images.h"
#include "Renderer/Vulkan/vk_initializers.h"
#include "Renderer/Vulkan/VulkanBuffer.h"

namespace Zero
{
    VulkanShadowmap::VulkanShadowmap()
    {
        CreateSampler();
        CreatePipeline();
        m_OffscreenImage = CreateImage(
            { SHADOW_WIDTH, SHADOW_HEIGHT, 1 },
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            false);
    }

    void VulkanShadowmap::DrawShadowMapTexture(Scene* scene, VkCommandBuffer cmd)
    {
        // Clear value for depth
        VkClearValue clearValue{};
        clearValue.depthStencil = {1.0f, 0};

        // Depth attachment info
        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = m_OffscreenImage.ImageView;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue = clearValue;

        // Rendering info (dynamic render pass)
        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.extent.width = SHADOW_WIDTH;
        renderingInfo.renderArea.extent.height = SHADOW_HEIGHT;
        renderingInfo.layerCount = 1;
        renderingInfo.pDepthAttachment = &depthAttachment;

        vkCmdBeginRendering(cmd, &renderingInfo);

        // Set viewport & scissor
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(SHADOW_WIDTH);
        viewport.height = static_cast<float>(SHADOW_HEIGHT);
        viewport.minDepth = 1.f;
        viewport.maxDepth = 0.f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = SHADOW_WIDTH;
        scissor.extent.height = SHADOW_HEIGHT;
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // Depth bias to avoid shadow acne
        vkCmdSetDepthBias(cmd, m_DepthBiasConstant, 0.0f, m_DepthBiasSlope);

        // Bind pipeline and descriptor sets
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_OffscreenPipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_OffscreenPipelineLayout, 0, 1,
                                &m_OffscreenDescriptorSet, 0, nullptr);

        GPUDrawPushConstants pushConstants{};

        for (const auto& gameObj : scene->GetGameObjects())
        {
            // GPUDrawPushConstants pushConstants{};
            pushConstants.ModelMatrix = gameObj->GetTransform().GetMatrix();
            pushConstants.CameraPos = Application::Get().GetActiveCamera().GetPosition();

            gameObj->GetModel()->Draw(cmd, m_OffscreenPipelineLayout, m_DepthSampler, pushConstants,
                                      gameObj->GetAnimator());
        }

        vkCmdEndRendering(cmd);
    }

    void VulkanShadowmap::CreateSampler()
    {
        VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        VK_CHECK(vkCreateSampler(renderer->GetDevice(), &samplerInfo, nullptr, &m_DepthSampler));
    }

    void VulkanShadowmap::CreatePipeline()
    {

    }

    void VulkanShadowmap::Destroy()
    {
        const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        if (!renderer)
        {
            printf("VulkanShadowmap::~VulkanShadowmap: Renderer is not valid");
            return;
        }

        vkDeviceWaitIdle(renderer->GetDevice());

        vkDestroyImageView(renderer->GetDevice(), m_OffscreenImage.ImageView, nullptr);
        vmaDestroyImage(renderer->GetAllocator(), m_OffscreenImage.Image, m_OffscreenImage.Allocation);
        vkDestroySampler(renderer->GetDevice(), m_DepthSampler, nullptr);
    }

    AllocatedImage VulkanShadowmap::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
                                                bool mipmapped)
    {
        VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        AllocatedImage newImage;
        newImage.ImageFormat = format;
        newImage.ImageExtent = size;

        VkImageCreateInfo imgInfo = VkInit::ImageCreateInfo(format, usage, size);
        if (mipmapped)
        {
            imgInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
        }

        // always allocate images on dedicated GPU memory
        VmaAllocationCreateInfo allocinfo = {};
        allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocinfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // allocate and create the image
        VK_CHECK(
            vmaCreateImage(renderer->GetAllocator(), &imgInfo, &allocinfo, &newImage.Image, &newImage.Allocation,
                nullptr));

        // if the format is a depth format, we will need to have it use the correct
        // aspect flag
        VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
        if (format == VK_FORMAT_D32_SFLOAT)
        {
            aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        // build an image-view for the image
        VkImageViewCreateInfo viewInfo = VkInit::ImageviewCreateInfo(format, newImage.Image, aspectFlag);
        viewInfo.subresourceRange.levelCount = imgInfo.mipLevels;

        VK_CHECK(vkCreateImageView(renderer->GetDevice(), &viewInfo, nullptr, &newImage.ImageView));

        return newImage;
    }

    AllocatedImage VulkanShadowmap::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
                                                bool mipmapped)
    {
        VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        const uint32_t dataSize = size.depth * size.width * size.height * 4;

        AllocatedBuffer uploadBuffer = VulkanBufferManager::CreateBuffer(
            renderer->GetAllocator(), dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU);

        memcpy(uploadBuffer.Info.pMappedData, data, dataSize);

        AllocatedImage new_image = CreateImage(size, format,
                                               usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

        renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
        {
            VkUtil::TransitionImage(cmd, new_image.Image, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VkBufferImageCopy copyRegion = {};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;

            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageExtent = size;

            // copy the buffer into the image
            vkCmdCopyBufferToImage(cmd, uploadBuffer.Buffer, new_image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                   &copyRegion);

            VkUtil::TransitionImage(cmd, new_image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        });

        VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), uploadBuffer);
        
        return new_image;
    }
}
