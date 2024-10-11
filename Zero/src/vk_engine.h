#pragma once

#include <shared/vk_types.h>
#include <GLFW/glfw3.h>

class VulkanEngine {
public:
	//initializes everything in the engine
	void init();
	//shuts down the engine
	void cleanup();
	//draw loop
	void draw();
	//run main loop
	void run();

	static VulkanEngine& Get();

private:
	void init_vulkan();
	void init_swapchain();
	void init_commands();
	void init_sync_structures();

	void create_swapchain(uint32_t width, uint32_t height);
	void destroy_swapchain();

	bool _isInitialized{ false };
	int _frameNumber{ 0 };
	bool stop_rendering{ false };
	VkExtent2D _windowExtent{ 1700 , 900 };

	GLFWwindow* _window;

	VkInstance _instance; // Vulkan API context, used to access drivers.
	VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
	VkPhysicalDevice _physicalDevice; // A GPU. Used to query physical GPU details, like features, capabilities, memory size, etc.
	VkDevice _device; // The “logical” GPU context that you actually execute things on.
	VkSurfaceKHR _surface; // Window surface.

	VkSwapchainKHR _swapchain;
	VkFormat _swapchainImageFormat;

	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	VkExtent2D _swapchainExtent;
};