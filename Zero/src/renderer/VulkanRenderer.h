#pragma once

#include <Renderer/Vulkan/vk_descriptors.h>
#include <Renderer/Vulkan/vk_types.h>
#include "renderer.h"
#include "core/core.h"

#include "glm/glm.hpp"
#define GLM_FORCE_RADIANS

#include <ranges>

#include <Model/Vulkan/VulkanMesh.h>
#include <Model/Vulkan/VulkanModel.h>
#include "Vulkan/VulkanTexture.h"
#include <Scene/Skybox/VulkanSkybox.h>

namespace Zero {

    struct DeletionQueue
    {
        std::deque<std::function<void()>> DeletionFunctions;

        void PushFunction(std::function<void()>&& function)
        {
            DeletionFunctions.push_back(function);
        }

        void Flush()
        {
            // reverse iterate the deletion queue to execute all the functions
            for (auto& deletionFunction : std::ranges::reverse_view(DeletionFunctions))
            {
                deletionFunction(); //call functors
            }

            DeletionFunctions.clear();
        }
    };

    struct FrameData
    {
        VkCommandPool CommandPool{};
        VkCommandBuffer MainCommandBuffer{};

        VkSemaphore SwapchainSemaphore{}, RenderSemaphore{};
        VkFence RenderFence{};

        DeletionQueue DeletionQueue;
        DescriptorAllocatorGrowable FrameDescriptors;
    };


    constexpr unsigned int FRAME_OVERLAP = 2;

    class VulkanRenderer : public Renderer
    {
    public:
        void Init() override;
        void InitImGui() override;
        void InitTextures();
        void Shutdown() override;
        void SetClearColor(const glm::vec4 clearColor) override { m_ClearColor = clearColor; }
        void Draw(Scene* scene) override;

        void DrawBackground(VkCommandBuffer cmd);
        void DrawGeometryTextured(Scene* scene, VkCommandBuffer cmd);

        void NewFrameImGui();
        void DrawImGui(VkCommandBuffer cmd, VkImageView targetImageView);

        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

        GPUMeshBuffers UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

        FrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; }

        VkDevice& GetDevice() { return m_Device; }
		VkPhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevice; }
		VkDescriptorSetLayout GetGpuSceneDataDescriptorLayout() const { return m_GpuSceneDataDescriptorLayout; }
        VkDescriptorSetLayout GetGameObjectDescriptorLayout() const { return m_GameObjectDescriptorLayout; }
        VmaAllocator& GetAllocator() { return m_Allocator; }
        DeletionQueue& GetMainDeletionQueue() { return m_MainDeletionQueue; }

        VulkanTexture& GetDefaultTexture() { return m_DefaultTexture; }

    private:
        void InitVulkan();
        void InitSwapchain();
        void InitCommands();
        void InitSyncStructures();
        void InitDescriptors();
        void InitPipelines();

        void InitTexturedPipeline();
		void InitSkyboxPipeline();

        void CreateSwapchain(uint32_t width, uint32_t height);
        void DestroySwapchain() const;
        void ResizeSwapchain();

        void DestroyImage(const AllocatedImage& image) const;

        int m_FrameNumber{0};
        VkExtent2D m_WindowExtent{EXTENT_WIDTH, EXTENT_HEIGHT};

        VkInstance m_Instance{}; // Vulkan API context, used to access drivers.
        VkDebugUtilsMessengerEXT m_DebugMessenger{}; // Vulkan debug output handle
        // A GPU. Used to query physical GPU details, like features, capabilities, memory size, etc.
        VkPhysicalDevice m_PhysicalDevice{};
        VkDevice m_Device{}; // The “logical” GPU context that you actually execute things on.
        VkSurfaceKHR m_Surface{}; // Window surface.

        VkSwapchainKHR m_Swapchain{};
        VkFormat m_SwapchainImageFormat{};

        glm::vec4 m_ClearColor{};

        std::vector<VkImage> m_SwapchainImages;
        std::vector<VkImageView> m_SwapchainImageViews;
        VkExtent2D m_SwapchainExtent{};

        DeletionQueue m_MainDeletionQueue;
        VmaAllocator m_Allocator{};

        AllocatedImage m_DrawImage{};
        AllocatedImage m_DepthImage{};
        VkExtent2D m_DrawExtent{};

        FrameData m_Frames[FRAME_OVERLAP] = {};

        VkQueue m_GraphicsQueue{};
        uint32_t m_GraphicsQueueFamily{};

        DescriptorAllocator m_GlobalDescriptorAllocator{};

        VkDescriptorSet m_DrawImageDescriptors{};
        VkDescriptorSetLayout m_DrawImageDescriptorLayout{};

        VkPipelineLayout m_SkyboxPipelineLayout{};
        VkPipeline m_SkyboxPipeline{};

        VkPipelineLayout m_TexturedPipelineLayout{};
        VkPipeline m_TexturedPipeline{};

        GPUMeshBuffers m_Rectangle{};

        VkFence m_ImmFence{};
        VkCommandBuffer m_ImmCommandBuffer{};
        VkCommandPool m_ImmCommandPool{};

        VulkanTexture m_DefaultTexture;

        VkSampler m_DefaultSamplerLinear{};
        VkSampler m_DefaultSamplerNearest{};

        VkDescriptorSetLayout m_GameObjectDescriptorLayout{};

        GPUSceneData m_SceneData{};
		GPUCameraData m_CameraData{};
        VkDescriptorSetLayout m_GpuSceneDataDescriptorLayout{};
    };
}
