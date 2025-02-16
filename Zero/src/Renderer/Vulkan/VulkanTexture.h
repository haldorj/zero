#pragma once
#include <Renderer/Vulkan/vk_types.h>

namespace Zero {

	class VulkanTexture
	{
		public:
		VulkanTexture() = default;
		VulkanTexture(std::string filepath, std::string type, bool mipmapped);

		AllocatedImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		AllocatedImage CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
			bool mipmapped = false);
		AllocatedImage CreateImageFromFile(const std::string& filePath, VkFormat format, VkImageUsageFlags usage,
			bool mipmapped);

		void DestroyImage();

		AllocatedImage CreateErrorImage();
		AllocatedImage GetImage() const { return m_Image; }

		std::string GetFilePath() const { return m_FilePath; }
		std::string GetType() const { return m_Type; }

	private:
		AllocatedImage m_Image;

		std::string m_FilePath;
		std::string m_Type;
	};

}
