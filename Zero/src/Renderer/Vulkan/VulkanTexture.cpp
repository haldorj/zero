#include "VulkanTexture.h"
#include "vk_initializers.h"

#include <Application.h>
#include "VulkanBuffer.h"
#include "vk_images.h"

namespace Zero {

    VulkanTexture::VulkanTexture(std::string filepath, std::string type, bool mipmapped)
    {
        m_FilePath = filepath;
        m_Type = type;
        m_Image = CreateImageFromFile(
            filepath, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, mipmapped);
    }

    AllocatedImage VulkanTexture::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
        bool mipmapped)
    {
        VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        AllocatedImage newImage;
        newImage.ImageFormat = format;
        newImage.ImageExtent = size;

        VkImageCreateInfo img_info = VkInit::ImageCreateInfo(format, usage, size);
        if (mipmapped)
        {
            img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
        }

        // always allocate images on dedicated GPU memory
        VmaAllocationCreateInfo allocinfo = {};
        allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocinfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // allocate and create the image
        VK_CHECK(vmaCreateImage(renderer->GetAllocator(), &img_info, &allocinfo, &newImage.Image, &newImage.Allocation, nullptr));

        // if the format is a depth format, we will need to have it use the correct
        // aspect flag
        VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
        if (format == VK_FORMAT_D32_SFLOAT)
        {
            aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        // build an image-view for the image
        VkImageViewCreateInfo viewInfo = VkInit::ImageviewCreateInfo(format, newImage.Image, aspectFlag);
        viewInfo.subresourceRange.levelCount = img_info.mipLevels;

        VK_CHECK(vkCreateImageView(renderer->GetDevice(), &viewInfo, nullptr, &newImage.ImageView));

        return newImage;
    }

    AllocatedImage VulkanTexture::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
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

    // Function to load an image from a file using stb_image and create an AllocatedImage
    AllocatedImage VulkanTexture::CreateImageFromFile(const std::string& filePath, VkFormat format, VkImageUsageFlags usage, bool mipmapped)
    {
        VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
        if (!renderer) 
            return CreateErrorImage();

        // Load image data using stb_image
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, 0);

        if (!pixels)
        {
            printf("Failed to load texture image! path: %s", filePath.c_str());
            return CreateErrorImage();
        }

        if (texChannels == 0)
        {
            return CreateErrorImage();
        }
        else if (texChannels == 1)
        {
            format = VK_FORMAT_R8G8B8_UNORM;
        }
        else if (texChannels == 2)
        {
            format = VK_FORMAT_R8G8_UNORM;
        }
        else if (texChannels == 3)
		{
			format = VK_FORMAT_R8G8B8_UNORM;
		}
		else if (texChannels == 4)
		{
			format = VK_FORMAT_R8G8B8A8_UNORM;
		}

        const VkExtent3D imageExtent = {
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight),
            1
        };

        // Calculate the size of the image data
        const VkDeviceSize imageSize = texWidth * texHeight * 4;

        // Create a staging buffer
        AllocatedBuffer stagingBuffer;
        try
        {
            stagingBuffer = VulkanBufferManager::CreateBuffer(renderer->GetAllocator(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        }
        catch (const std::exception& e)
        {
            stbi_image_free(pixels);  // Free image data in case of failure
            throw;  // Rethrow the exception
        }

        // Copy image data to the staging buffer
        void* data;
        vmaMapMemory(renderer->GetAllocator(), stagingBuffer.Allocation, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vmaUnmapMemory(renderer->GetAllocator(), stagingBuffer.Allocation);

        // Free the image data loaded by stb_image
        stbi_image_free(pixels);

        // Create the Vulkan image
        AllocatedImage newImage;
        try
        {
            newImage = CreateImage(imageExtent, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, mipmapped);
        }
        catch (const std::exception& e)
        {
            VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), stagingBuffer);  // Clean up staging buffer on failure
            throw;  // Rethrow the exception
        }

        renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
            {
                VkUtil::TransitionImage(cmd, newImage.Image, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                VkBufferImageCopy copyRegion = {};
                copyRegion.bufferOffset = 0;
                copyRegion.bufferRowLength = 0;
                copyRegion.bufferImageHeight = 0;

                copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.imageSubresource.mipLevel = 0;
                copyRegion.imageSubresource.baseArrayLayer = 0;
                copyRegion.imageSubresource.layerCount = 1;
                copyRegion.imageExtent = imageExtent;

                // Copy the buffer into the image
                vkCmdCopyBufferToImage(cmd, stagingBuffer.Buffer, newImage.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                    &copyRegion);

                VkUtil::TransitionImage(cmd, newImage.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            });

        // Clean up the staging buffer
        VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), stagingBuffer);

        return newImage;
    }

    void VulkanTexture::DestroyImage()
    {
        VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
        vkDeviceWaitIdle(renderer->GetDevice());
        
		vkDestroyImageView(renderer->GetDevice(), m_Image.ImageView, nullptr);
		vmaDestroyImage(renderer->GetAllocator(), m_Image.Image, m_Image.Allocation);
    }

    AllocatedImage VulkanTexture::CreateErrorImage()
    {
        // Checkerboard image
        const uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
        const uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
        std::array<uint32_t, static_cast<uint32_t>(16 * 16)> pixels; //for 16x16 checkerboard texture
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
            }
        }
        return CreateImage(pixels.data(), { 16, 16, 1 }, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, false);
    }
}