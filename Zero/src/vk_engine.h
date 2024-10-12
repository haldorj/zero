#pragma once

#include <shared/vk_types.h>
#include <GLFW/glfw3.h>
#include "renderer/Vk_Renderer.h"

enum class RendererType {
	OpenGL,
	Vulkan
};

class RendererFactory {
public:
	static Renderer* CreateRenderer(RendererType type) {
		switch (type) {
		case RendererType::OpenGL:
			// return new OpenGLRenderer();
		case RendererType::Vulkan:
			return new Vk_Renderer();
		default:
			return nullptr;
		}
	}
};

class Engine {
public:
	//initializes everything in the engine
	void init();
	//shuts down the engine
	void cleanup();
	//draw loop
	void draw();
	//run main loop
	void run();

	static Engine& Get();

	GLFWwindow* GetWindow() const { return _window; }
	

private:
	bool _isInitialized{ false };
	bool stop_rendering{ false };

	GLFWwindow* _window;
	Renderer* _renderer;
};