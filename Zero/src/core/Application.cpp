#include "Application.h"
#include "core/core.h"

#include <Renderer/Vulkan/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>
#include "model/Vertex.h"

namespace Zero
{
    // Choose RendererAPI
    RendererAPI RendererType = RendererAPI::Vulkan;

    Application* LoadedEngine = nullptr;
    Application& Application::Get() { return *LoadedEngine; }

    void Application::CreateRectangle() const
    {
        std::array<Vertex, 4> rectVertices{
            Vertex{{0.5, -0.5, 0}, {0.2, 0.8, 0.2}, {1, 0}}, // 0
            Vertex{{0.5, 0.5, 0}, {0.8, 0.8, 0.2}, {1, 1}}, // 1
            Vertex{{-0.5, -0.5, 0}, {0.8, 0.2, 0.2}, {0, 0}}, // 2
            Vertex{{-0.5, 0.5, 0}, {0.2, 0.2, 0.8}, {0, 1}} // 3
        };

        std::array<uint32_t, 6> rectIndices{};

        rectIndices[0] = 0;
        rectIndices[1] = 1;
        rectIndices[2] = 2;

        rectIndices[3] = 2;
        rectIndices[4] = 1;
        rectIndices[5] = 3;

        m_Renderer->InitObject(rectIndices, rectVertices);
    }

    void Application::CreatePyramid() const
    {
        std::array<Vertex, 5> pyramidVertices{
            Vertex{{-0.5, 0.0, 0.5}, {1, 0, 0}, {0, 0}}, // 0
            Vertex{{-0.5, 0.0, -0.5}, {0, 1, 0}, {5, 0}}, // 1
            Vertex{{0.5, 0.0, -0.5}, {0, 0, 1}, {0, 0}}, // 2
            Vertex{{0.5, 0.0, 0.5}, {1, 1, 0}, {5, 0}}, // 3
            Vertex{{0, 0.8, 0}, {1, 1, 1}, {2.5, 5}} // 4
        };

        std::array<uint32_t, 18> indices =
        {
            0, 1, 2,
            0, 2, 3,
            0, 1, 4,
            1, 2, 4,
            2, 3, 4,
            3, 0, 4
        };

        m_Renderer->InitObject(indices, pyramidVertices);
    }

    void Application::Init()
    {
        // Only one engine initialization is allowed with the application.
        assert(LoadedEngine == nullptr);
        LoadedEngine = this;

        InitGLFW(RendererType);

        // Initialize the renderer
        m_Renderer = RendererFactory::CreateRenderer(RendererType);

        m_MainCamera.SetPosition({0, 0.5, -2});

        switch (RendererType)
        {
        case RendererAPI::OpenGL:
            CreatePyramid();
            m_Renderer->Init();
            break;
        case RendererAPI::Vulkan:
            m_Renderer->Init();
            CreatePyramid();
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
