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
	bool _isInitialized{ false };
	int _frameNumber{ 0 };
	bool stop_rendering{ false };
	VkExtent2D _windowExtent{ 1700 , 900 };

	GLFWwindow* _window;
};