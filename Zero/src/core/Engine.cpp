#include "Engine.h"
#include "core/core.h"

#include <shared/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>

// Choose RendererAPI
RendererAPI rendererType = RendererAPI::Vulkan;

Engine* loadedEngine = nullptr;
Engine& Engine::Get() { return *loadedEngine; }

void Engine::CreateRectangle()
{
    std::array<Vertex, 4> rect_vertices{};

    rect_vertices[0].position = { 0.5,-0.5, 0 };
    rect_vertices[1].position = { 0.5,0.5, 0 };
    rect_vertices[2].position = { -0.5,-0.5, 0 };
    rect_vertices[3].position = { -0.5,0.5, 0 };

    rect_vertices[0].color = { .2, .8, .2, 1 };
    rect_vertices[1].color = { .8, .8, .2, 1 };
    rect_vertices[2].color = { .8, .2, .2, 1 };
    rect_vertices[3].color = { .2, .2, .8, 1 };

    std::array<uint32_t, 6> rect_indices{};

    rect_indices[0] = 0;
    rect_indices[1] = 1;
    rect_indices[2] = 2;

    rect_indices[3] = 2;
    rect_indices[4] = 1;
    rect_indices[5] = 3;

    _renderer->InitObject(rect_indices, rect_vertices);
}

void Engine::Init()
{
    // Only one engine initialization is allowed with the application.
    assert(loadedEngine == nullptr);
    loadedEngine = this;

    InitGLFW(rendererType);

    // Initialize the renderer
    _renderer = RendererFactory::CreateRenderer(rendererType);
    
    switch (rendererType)
    {
    case RendererAPI::OpenGL:
        CreateRectangle();
        _renderer->Init();
        break;
    case RendererAPI::Vulkan:
        _renderer->Init();
        CreateRectangle();
        break;
    default:
        break;
    }

    // everything went fine
    _isInitialized = true;
}

void Engine::Cleanup()
{
    if (_isInitialized)
    {
        _renderer->Shutdown();

        glfwDestroyWindow(_window);
        glfwTerminate();
    }

    loadedEngine = nullptr;
}

void Engine::Draw()
{
    float flash = std::abs(std::sin(_frameNumber / 240.f));

    _renderer->SetClearColor({ 0, 0, flash * 0.5, 1});
    _renderer->Draw();

    _frameNumber++;
}

void Engine::Run()
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

        Draw();
    }
}

void Engine::InitGLFW(RendererAPI rendererType)
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
