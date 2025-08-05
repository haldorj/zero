#pragma once
#include "Renderer/Vulkan/vk_types.h"
#include "Renderer/Vulkan/vk_descriptors.h"

namespace Zero {
    class Scene;

    class VulkanShadowmap
    {
    public:
        VulkanShadowmap();
        
        void DrawShadowMapTexture(Scene* scene, VkCommandBuffer cmd, DescriptorWriter& descriptorWriter);
        AllocatedImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
        AllocatedImage CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
            bool mipmapped = false);

        void CreateSampler();
        void CreatePipeline();

		AllocatedImage GetOffscreenImage() const { return m_OffscreenImage; }

        void Destroy() const;
    private:
        AllocatedImage m_OffscreenImage{};
        VkPipelineLayout m_OffscreenPipelineLayout{};
        VkPipeline m_OffscreenPipeline{};
        VkSampler m_DepthSampler{};
    };
}
