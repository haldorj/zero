#pragma once

#include <shared/vk_types.h>
#include <GLFW/glfw3.h>

#include "camera/Camera.h"
#include "EnumModes/RendererMode.hpp"
#include "renderer/VulkanRenderer.h"
#include "renderer/OpenGLRenderer.h"

namespace Zero
{
    enum class RendererAPI
    {
        OpenGL,
        Vulkan
    };

    class RendererFactory
    {
    public:
        static RendererBase* CreateRenderer(const RendererAPI type)
        {
            switch (type)
            {
            case RendererAPI::OpenGL:
                return new OpenGLRenderer();
            case RendererAPI::Vulkan:
                return new VulkanRenderer();
            }
            return nullptr;
        }
    };
}

namespace Zero
{
    class Application
    {
    public:
        void CreateRectangle() const;
        void CreatePyramid() const;
        //initializes everything in the engine
        void Init();
        //shuts down the engine
        void Cleanup() const;
        //draw loop
        void Draw();
        //run main loop
        void Run();

        void InitGLFW(RendererAPI rendererType);

        static Application& Get();

        GLFWwindow* GetWindow() const { return m_Window; }
        Camera& GetMainCamera() { return m_MainCamera; }

    private:
        Camera m_MainCamera{};

        bool m_IsInitialized{false};
        bool m_StopRendering{false};

        int32_t m_FrameCount{0};

        GLFWwindow* m_Window = nullptr;
        RendererBase* m_Renderer = nullptr;

        Topology m_topology = Topology::Lines;
        void SetRendererMode(const Topology& tp) { m_topology = tp; }
    };
} // namespace Zero
