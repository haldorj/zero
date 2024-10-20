#include "Application.h"
#include "core/core.h"

#include <shared/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>

namespace Zero
{
    // Choose RendererAPI
    RendererAPI RendererType = RendererAPI::Vulkan;

    Application* LoadedEngine = nullptr;
    Application& Application::Get() { return *LoadedEngine; }

    void Application::CreateRectangle() const
    {
        std::array<Vertex, 4> rectVertices{};

        rectVertices[0].Position = {0.5, -0.5, 0};
        rectVertices[0].UvX = 1; rectVertices[0].UvY = 0;
        rectVertices[1].Position = {0.5, 0.5, 0};
        rectVertices[1].UvX = 1; rectVertices[1].UvY = 1;
        rectVertices[2].Position = {-0.5, -0.5, 0};
        rectVertices[2].UvX = 0; rectVertices[2].UvY = 0;
        rectVertices[3].Position = {-0.5, 0.5, 0};
        rectVertices[3].UvX = 0; rectVertices[3].UvY = 1;

        rectVertices[0].Color = {.2, .8, .2, 1};
        rectVertices[1].Color = {.8, .8, .2, 1};
        rectVertices[2].Color = {.8, .2, .2, 1};
        rectVertices[3].Color = {.2, .2, .8, 1};

        std::array<uint32_t, 6> rectIndices{};

        rectIndices[0] = 0;
        rectIndices[1] = 1;
        rectIndices[2] = 2;

        rectIndices[3] = 2;
        rectIndices[4] = 1;
        rectIndices[5] = 3;

        m_Renderer->InitObject(rectIndices, rectVertices);
    }

    void Application::Init()
    {
        // Only one engine initialization is allowed with the application.
        assert(LoadedEngine == nullptr);
        LoadedEngine = this;

        InitGLFW(RendererType);

        // Initialize the renderer
        m_Renderer = RendererFactory::CreateRenderer(RendererType);

        switch (RendererType)
        {
        case RendererAPI::OpenGL:
            CreateRectangle();
            m_Renderer->Init();
            break;
        case RendererAPI::Vulkan:
            m_Renderer->Init();
            CreateRectangle();
            break;
        }

        // everything went fine
        m_IsInitialized = true;
    }

    void Application::Cleanup() const
    {
        if (m_IsInitialized)
        {
            m_Renderer->Shutdown();

            glfwDestroyWindow(m_Window);
            glfwTerminate();
        }

        LoadedEngine = nullptr;
    }

    void Application::Draw()
    {
        const float flash = std::abs(std::sin(static_cast<float>(m_FrameCount) / 240.f));

        m_Renderer->SetClearColor({0, 0, flash * 0.5, 1});
        m_Renderer->Draw();

        m_FrameCount++;
    }

    void Application::Run()
    {
        // main loop
        while (!glfwWindowShouldClose(m_Window))
        {
            // Poll and handle events
            glfwPollEvents();

            // Check if the window is minimized
            if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            m_MainCamera.ProcessInput(m_Window, 0.02f);
            m_MainCamera.Update();
            Draw();
        }
    }

    void Application::InitGLFW(const RendererAPI rendererType)
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
        m_Window = glfwCreateWindow(EXTENT_WIDTH, EXTENT_HEIGHT, "ZeroEngine", nullptr, nullptr);
        if (!m_Window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }
    }
} // namespace Zero
