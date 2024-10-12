#pragma once

#include "renderer.h"
#include "core/core.h"
#include <shared/vk_types.h>

struct AllocatedImage {
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
};

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call functors
		}

		deletors.clear();
	}
};

struct FrameData {

	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	VkSemaphore _swapchainSemaphore, _renderSemaphore;
	VkFence _renderFence;

	DeletionQueue _deletionQueue;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class Vk_Renderer : public Renderer
{
public:
    void Init() override;
    void Shutdown() override;
	void SetClearColor(glm::vec4 clearColor) override { _clearColor = clearColor; }
    void Draw() override;

	void DrawBackground(VkCommandBuffer cmd);

	FrameData _frames[FRAME_OVERLAP];

	FrameData& get_current_frame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

	VkQueue _graphicsQueue;
	uint32_t _graphicsQueueFamily;

private:
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_sync_structures();

	void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();

	int _frameNumber{ 0 };
	VkExtent2D _windowExtent{ WIDTH , HEIGHT };

	VkInstance _instance; // Vulkan API context, used to access drivers.
	VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
	VkPhysicalDevice _physicalDevice; // A GPU. Used to query physical GPU details, like features, capabilities, memory size, etc.
	VkDevice _device; // The “logical” GPU context that you actually execute things on.
	VkSurfaceKHR _surface; // Window surface.

	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat;

	glm::vec4 _clearColor;

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	VkExtent2D _swapchainExtent;

	DeletionQueue _mainDeletionQueue;
	VmaAllocator _allocator;

	AllocatedImage _drawImage;
	VkExtent2D _drawExtent;
};

