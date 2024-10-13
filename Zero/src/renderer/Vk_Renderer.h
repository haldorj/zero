#pragma once

#include "renderer.h"
#include "core/core.h"
#include <shared/vk_types.h>
#include <shared/vk_descriptors.h>

struct AllocatedImage {
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};

struct DeletionQueue
{
    std::deque<std::function<void()>> deletionFunctions;

    void push_function(std::function<void()>&& function) {
        deletionFunctions.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletionFunctions.rbegin(); it != deletionFunctions.rend(); it++) {
            (*it)(); //call functors
        }

        deletionFunctions.clear();
    }
};

struct FrameData 
{
	VkCommandPool _commandPool{};
	VkCommandBuffer _mainCommandBuffer{};

	VkSemaphore _swapchainSemaphore{}, _renderSemaphore{};
	VkFence _renderFence{};

	DeletionQueue _deletionQueue;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class Vk_Renderer : public RendererBase
{
public:
    void Init() override;
	void init_default_data();
    void Shutdown() override;
	void SetClearColor(glm::vec4 clearColor) override { _clearColor = clearColor; }
    void Draw() override;

	void DrawBackground(VkCommandBuffer cmd);
	void DrawComputeBackground(VkCommandBuffer cmd);
	void DrawGeometry(VkCommandBuffer cmd);

	FrameData _frames[FRAME_OVERLAP];

	FrameData& get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

	VkQueue _graphicsQueue{};
	uint32_t _graphicsQueueFamily{};

	DescriptorAllocator globalDescriptorAllocator;

	VkDescriptorSet _drawImageDescriptors;
	VkDescriptorSetLayout _drawImageDescriptorLayout;

	VkPipeline _gradientPipeline;
	VkPipelineLayout _gradientPipelineLayout;
	
	VkPipeline _plainPipeline;
	VkPipelineLayout _plainPipelineLayout;

	VkPipelineLayout _texturedPipelineLayout;
	VkPipeline _texturedPipeline;

	GPUMeshBuffers rectangle;


	VkFence _immFence;
	VkCommandBuffer _immCommandBuffer;
	VkCommandPool _immCommandPool;



private:
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_sync_structures();
	void init_descriptors();
	void init_pipelines();

	void init_background_pipelines();
	void init_plain_pipeline();
	void init_textured_pipeline();

	void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();

	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void destroy_buffer(const AllocatedBuffer& buffer);

	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

	int _frameNumber{ 0 };
	VkExtent2D _windowExtent{ EXTENT_WIDTH , EXTENT_HEIGHT };

	VkInstance _instance{}; // Vulkan API context, used to access drivers.
	VkDebugUtilsMessengerEXT _debug_messenger{}; // Vulkan debug output handle
	VkPhysicalDevice _physicalDevice{}; // A GPU. Used to query physical GPU details, like features, capabilities, memory size, etc.
	VkDevice _device{}; // The “logical” GPU context that you actually execute things on.
	VkSurfaceKHR _surface{}; // Window surface.

	VkSwapchainKHR _swapchain{};
	VkFormat _swapchainImageFormat{};

	glm::vec4 _clearColor{};

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	VkExtent2D _swapchainExtent{};

	DeletionQueue _mainDeletionQueue;
	VmaAllocator _allocator{};

	AllocatedImage _drawImage{};
	VkExtent2D _drawExtent{};


};

