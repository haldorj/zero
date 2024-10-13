#include "Engine.h"
#include "core/core.h"

#include <shared/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>


Engine* loadedEngine = nullptr;

Engine& Engine::Get() { return *loadedEngine; }

void Engine::init()
{
    // Only one engine initialization is allowed with the application.
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    // Choose renderer backend!
    const RendererAPI rendererType = RendererAPI::Vulkan;

    initGLFW(rendererType);

    // Initialize the renderer
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
    float flash = std::abs(std::sin(_frameNumber / 240.f));

    _renderer->SetClearColor({ 0, 0, flash * 0.5, 1});
    _renderer->Draw();

    _frameNumber++;
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

void Engine::initGLFW(RendererAPI rendererType)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    if (rendererType == RendererAPI::Vulkan)
    {
        // Set GLFW to not create an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
    if (rendererType == RendererAPI::OpenGL)
    {
        // Tell GLFW what version of OpenGL we are using 
        // In this case we are using OpenGL 4.6
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        // Tell GLFW we are using the CORE profile
        // So that means we only have the modern functions
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
    // Create a GLFW window
    _window = glfwCreateWindow(EXTENT_WIDTH, EXTENT_HEIGHT, "ZeroEngine", nullptr, nullptr);
    if (!_window)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
}
