#pragma once

#include "renderer.h"
#include "core/core.h"
#include <shared/vk_types.h>
#include <shared/vk_descriptors.h>

#include <ranges>

struct AllocatedImage {
	VkImage Image;
	VkImageView ImageView;
	VmaAllocation Allocation;
	VkExtent3D ImageExtent;
	VkFormat ImageFormat;
};

struct DeletionQueue
{
    std::deque<std::function<void()>> DeletionFunctions;

    void PushFunction(std::function<void()>&& function) {
        DeletionFunctions.push_back(function);
    }

    void Flush() {
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
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanRenderer : public RendererBase
{
public:
    void Init() override;
	void InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices) override;
    void Shutdown() override;
	void SetClearColor(const glm::vec4 clearColor) override { m_ClearColor = clearColor; }
    void Draw() override;

	void DrawBackground(VkCommandBuffer cmd);
	void DrawComputeBackground(VkCommandBuffer cmd);
	void DrawGeometry(VkCommandBuffer cmd);

	GPUMeshBuffers UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

private:
	void InitVulkan();
	void InitSwapchain();
	void InitCommands();
	void InitSyncStructures();
	void InitDescriptors();
	void InitPipelines();

	void InitBackgroundPipelines();
	void InitPlainPipeline();
	void InitTexturedPipeline();

	void CreateSwapchain(uint32_t width, uint32_t height);
	void DestroySwapchain();
	void ResizeSwapchain();

	AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void DestroyBuffer(const AllocatedBuffer& buffer);

	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	int m_FrameNumber{ 0 };
	VkExtent2D m_WindowExtent{ EXTENT_WIDTH , EXTENT_HEIGHT };

	VkInstance m_Instance{}; // Vulkan API context, used to access drivers.
	VkDebugUtilsMessengerEXT m_DebugMessenger{}; // Vulkan debug output handle
	VkPhysicalDevice m_PhysicalDevice{}; // A GPU. Used to query physical GPU details, like features, capabilities, memory size, etc.
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
	VkExtent2D m_DrawExtent{};

	FrameData m_Frames[FRAME_OVERLAP];

	FrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; };

	VkQueue m_GraphicsQueue{};
	uint32_t m_GraphicsQueueFamily{};

	DescriptorAllocator m_GlobalDescriptorAllocator;

	VkDescriptorSet m_DrawImageDescriptors;
	VkDescriptorSetLayout m_DrawImageDescriptorLayout;

	VkPipeline m_GradientPipeline;
	VkPipelineLayout m_GradientPipelineLayout;
	
	VkPipeline m_PlainPipeline;
	VkPipelineLayout m_PlainPipelineLayout;

	VkPipelineLayout m_TexturedPipelineLayout;
	VkPipeline m_TexturedPipeline;

	GPUMeshBuffers m_Rectangle;

	VkFence m_ImmFence;
	VkCommandBuffer m_ImmCommandBuffer;
	VkCommandPool m_ImmCommandPool;
};

