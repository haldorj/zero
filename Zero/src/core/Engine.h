#pragma once

#include <shared/vk_types.h>
#include <GLFW/glfw3.h>
#include "renderer/Vk_Renderer.h"
#include "renderer/OpenGLRenderer.h"

enum class RendererAPI {
	OpenGL,
	Vulkan
};

class RendererFactory {
public:
	static RendererBase* CreateRenderer(RendererAPI type) {
		switch (type) {
		case RendererAPI::OpenGL:
			return new OpenGLRenderer();
		case RendererAPI::Vulkan:
			return new Vk_Renderer();
		default:
			return nullptr;
		}
	}
};

class Engine {
public:
	void CreateRectangle();
	//initializes everything in the engine
	void Init();
	//shuts down the engine
	void Cleanup();
	//draw loop
	void Draw();
	//run main loop
	void Run();

	void InitGLFW(RendererAPI rendererType);

	static Engine& Get();

	GLFWwindow* GetWindow() const { return _window; }

private:
	bool _isInitialized{ false };
	bool stop_rendering{ false };

	int32_t _frameNumber{ 0 };

	GLFWwindow* _window;
	RendererBase* _renderer;
};