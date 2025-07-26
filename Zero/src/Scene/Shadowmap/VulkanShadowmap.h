#pragma once
#include "Renderer/Vulkan/vk_types.h"

namespace Zero {
    class Scene;

    class VulkanShadowmap
    {
    public:
        VulkanShadowmap();
        
        void DrawShadowMapTexture(Scene* scene, VkCommandBuffer cmd);
        AllocatedImage CreateImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
        AllocatedImage CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
            bool mipmapped = false);

        void CreateSampler();
        void CreatePipeline();
    private:
        // Shadow mapping
        VkPipelineLayout m_OffscreenPipelineLayout{};
        VkPipeline m_OffscreenPipeline{};
        VkDescriptorSet m_OffscreenDescriptorSet{};
        AllocatedImage m_OffscreenImage{};

        VkSampler m_DepthSampler{};
        
        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        float m_DepthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        float m_DepthBiasSlope = 1.75f;
    };
}
