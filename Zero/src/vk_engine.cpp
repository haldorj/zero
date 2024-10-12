#include "vk_engine.h"

#include <shared/vk_initializers.h>
#include <shared/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <shared/vk_images.h>


Engine* loadedEngine = nullptr;

Engine& Engine::Get() { return *loadedEngine; }

void Engine::init()
{
    // Only one engine initialization is allowed with the application.
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    // Initialize GLFW
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Set GLFW to not create an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create a GLFW window
    _window = glfwCreateWindow(1700, 900, "Vulkan Engine", nullptr, nullptr);
    if (!_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Initialize the renderer
    RendererType rendererType = RendererType::Vulkan;
    _renderer = RendererFactory::CreateRenderer(rendererType);
    _renderer->Init();

    // everything went fine
    _isInitialized = true;
}

void Engine::cleanup()
{
    if (_isInitialized)
    {
        _renderer->Shutdown();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    loadedEngine = nullptr;
}

void Engine::draw()
{
    _renderer->SetClearColor({.1, 0.03, 0.2f, 1});
    _renderer->Draw();
}

void Engine::run()
{
    // main loop
    while (!glfwWindowShouldClose(_window))
    {
        // Poll and handle events
        glfwPollEvents();

        // Check if the window is minimized
        if (glfwGetWindowAttrib(_window, GLFW_ICONIFIED))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        draw();
    }
}
