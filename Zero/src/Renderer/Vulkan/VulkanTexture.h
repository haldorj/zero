#pragma once
#include <Renderer/Vulkan/vk_types.h>

class VulkanTexture
{
	public:
	VulkanTexture();
	~VulkanTexture();

	AllocatedImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
	AllocatedImage CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
		bool mipmapped = false);
	static AllocatedImage CreateImageFromFile(const std::string& filePath, VkFormat format, VkImageUsageFlags usage,
		bool mipmapped);

private:
	AllocatedImage m_Image;

	std::string m_FilePath;
	std::string m_Type;
};

