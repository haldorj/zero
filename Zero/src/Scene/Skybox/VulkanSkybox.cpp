#include "VulkanSkybox.h"

#include <stb_image.h>
#include <Application.h>
#include <Renderer/Vulkan/VulkanBuffer.h>
#include <Renderer/Vulkan/vk_initializers.h>
#include <Renderer/Vulkan/vk_images.h>

namespace Zero {

	VulkanSkybox::VulkanSkybox(std::vector<std::string> faceLocations)
	{
		CreateSkyboxMesh();
		CreateSampler();
		m_Image = CreateCubeMap(faceLocations);
	}

	void VulkanSkybox::Draw(const VkCommandBuffer& cmd, const VkPipelineLayout& pipelineLayout,
		VkExtent2D drawExtent, GPUDrawPushConstants& pushConstants,
		DescriptorWriter& descriptorWriter)
	{
		const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
		if (!renderer)
		{
			printf("VulkanSkybox::Draw: Renderer is not of type VulkanRenderer");
			return;
		}

		const VkDescriptorSet descriptorSet = renderer->GetCurrentFrame().FrameDescriptors.Allocate(
			renderer->GetDevice(), renderer->GetGpuSceneDataDescriptorLayout());

		descriptorWriter.WriteImage(
			2, 
			m_Image.ImageView, 
			m_Sampler,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		);

		descriptorWriter.UpdateSet(renderer->GetDevice(), descriptorSet);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		
		pushConstants.VertexBuffer = m_GPUMeshBuffers.VertexBufferAddress;

		vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants),
			&pushConstants);
		vkCmdBindIndexBuffer(cmd, m_GPUMeshBuffers.IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(cmd, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
	}

	void VulkanSkybox::Destroy() const
	{
		const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

		if (!renderer)
		{
			printf("VulkanSkybox::~VulkanSkybox: Renderer is not of type VulkanRenderer");
			return;
		}

		vkDeviceWaitIdle(renderer->GetDevice());

		vkDestroyImageView(renderer->GetDevice(), m_Image.ImageView, nullptr);
		vmaDestroyImage(renderer->GetAllocator(), m_Image.Image, m_Image.Allocation);
		vkDestroySampler(renderer->GetDevice(), m_Sampler, nullptr);
		VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), m_GPUMeshBuffers.IndexBuffer);
		VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), m_GPUMeshBuffers.VertexBuffer);
	}

	AllocatedImage VulkanSkybox::CreateCubeMap(const std::vector<std::string>& faceLocations)
	{
		VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

		constexpr size_t numFaces = 6;

		int texWidth, texHeight, texChannels;
		stbi_uc* pixels[numFaces]{};

		for (int i = 0; i < numFaces; ++i)
		{
			pixels[i] = stbi_load(faceLocations[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			if (!pixels[i])
			{
				throw std::runtime_error("Failed to load cubemap face: " + faceLocations[i]);
			}
		}

		const VkExtent3D imageExtent = {
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			1
		};

		const VkDeviceSize layerSize = texWidth * texHeight * 4;
		const VkDeviceSize imageSize = layerSize * numFaces;

		AllocatedBuffer stagingBuffer;
		stagingBuffer = VulkanBufferManager::CreateBuffer(renderer->GetAllocator(), imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		// Copy pixel data into staging buffer
		void* data;
		vmaMapMemory(renderer->GetAllocator(), stagingBuffer.Allocation, &data);
		for (int i = 0; i < numFaces; ++i)
		{
			memcpy(static_cast<char*>(data) + i * layerSize, pixels[i], layerSize);
		}
		vmaUnmapMemory(renderer->GetAllocator(), stagingBuffer.Allocation);

		// Free the image data loaded by stb_image
		for (int i = 0; i < numFaces; ++i)
		{
			stbi_image_free(pixels[i]);
		}

		// Create the Vulkan image
		AllocatedImage newImage;

		newImage = CreateImage(imageExtent, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, false);
		
		renderer->ImmediateSubmit([&](VkCommandBuffer cmd)

		{
			VkUtil::TransitionImageCubemap(
				cmd, 
				newImage.Image, 
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			// Prepare 6-layer copy
			std::vector<VkBufferImageCopy> copyRegions;
			for (uint32_t face = 0; face < numFaces; ++face)
			{
				VkBufferImageCopy region{};
				region.bufferOffset = layerSize * face;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = 0;
				region.imageSubresource.baseArrayLayer = face;
				region.imageSubresource.layerCount = 1;
				region.imageExtent = imageExtent;

				copyRegions.emplace_back(region);
			}

			// Copy the buffer into the image
			vkCmdCopyBufferToImage(
				cmd, 
				stagingBuffer.Buffer, 
				newImage.Image, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				static_cast<uint32_t>(copyRegions.size()),
				copyRegions.data()
			);

			VkUtil::TransitionImageCubemap(
				cmd, 
				newImage.Image, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		});

		// Clean up the staging buffer
		VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), stagingBuffer);

		return newImage;
	}

	AllocatedImage VulkanSkybox::CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
		bool mipmapped)
	{
		VulkanRenderer* renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

		AllocatedImage newImage;
		newImage.ImageFormat = format;
		newImage.ImageExtent = size;

		VkImageCreateInfo img_info = VkInit::CubemapImageCreateInfo(format, usage, size);
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
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE; // Set view type for cubemap (samplerCube uniform in glsl)
		viewInfo.subresourceRange.levelCount = img_info.mipLevels;
		viewInfo.subresourceRange.layerCount = 6;

		VK_CHECK(vkCreateImageView(renderer->GetDevice(), &viewInfo, nullptr, &newImage.ImageView));

		return newImage;
	}

	void VulkanSkybox::CreateSkyboxMesh()
	{
		const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());
		if (!renderer)
		{
			printf("VulkanSkybox::CreateSkyboxMesh: Renderer is not of type VulkanRenderer");
			return;
		}

		m_Indices.reserve(36);
		m_Indices = {
			// front
			0, 1, 2,
			2, 1, 3,
			// right
			2, 3, 5,
			5, 3, 7,
			// back
			5, 7, 4,
			4, 7, 6,
			// left
			4, 6, 0,
			0, 6, 1,
			// top
			4, 0, 5,
			5, 0, 2,
			// bottom
			1, 6, 3,
			3, 6, 7
		};

		m_Vertices.reserve(8);

		std::array<Vertex, 8> vertices{};

		vertices[0].Position = {-1.0f, 1.0f, -1.0f};
		vertices[1].Position = { -1.0f, -1.0f, -1.0f };
		vertices[2].Position = { 1.0f, 1.0f, -1.0f };
		vertices[3].Position = { 1.0f, -1.0f, -1.0f };
		vertices[4].Position = { -1.0f, 1.0f, 1.0f };
		vertices[5].Position = { 1.0f, 1.0f, 1.0f };
		vertices[6].Position = { -1.0f, -1.0f, 1.0f };
		vertices[7].Position = { 1.0f, -1.0f, 1.0f };

		for (size_t i = 0; i < vertices.size(); ++i)
		{
			m_Vertices.emplace_back(vertices[i]);
		}

		m_GPUMeshBuffers = renderer->UploadMesh(m_Indices, m_Vertices);		
	}

	void VulkanSkybox::CreateSampler()
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
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.maxAnisotropy = 1.0f;

		VkPhysicalDeviceFeatures features = {};
		vkGetPhysicalDeviceFeatures(renderer->GetPhysicalDevice(), &features);

		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(renderer->GetPhysicalDevice(), &properties);

		//if (features.samplerAnisotropy)
		//{
		//	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		//	samplerInfo.anisotropyEnable = VK_TRUE;
		//}

		VK_CHECK(vkCreateSampler(renderer->GetDevice(), &samplerInfo, nullptr, &m_Sampler));
	}

}