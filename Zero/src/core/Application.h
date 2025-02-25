#pragma once

#include <GLFW/glfw3.h>
#include <Renderer/Vulkan/vk_types.h>

#include "camera/Camera.h"
#include "renderer/OpenGLRenderer.h"
#include "renderer/VulkanRenderer.h"

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
        // Reminder: 
        // If you get the error "attempting to reference a deleted function"
        // Remember to give new objects a default constructor
        static Renderer* CreateRenderer(const RendererAPI type)
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

    class ModelFactory
    {
    public:
        static std::shared_ptr<Model> CreateModel(const char* path, const RendererAPI type)
        {
            switch (type)
            {
            case RendererAPI::OpenGL: return std::make_shared<OpenGLModel>(path);
            case RendererAPI::Vulkan: return std::make_shared<VulkanModel>(path);
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
        //initializes everything in the engine
        void Init();
        //shuts down the engine
        void Cleanup() const;
        //draw loop
        void Draw();
        void UpdateImGui();
        //run main loop
        void Run();

        void InitGLFW(RendererAPI rendererType);

        static float GetRandomFloat(float min, float max);

        void InitGameObjects();

        void SpawnSphere();
        void DestroyGameObject(GameObject::IdT objectID);

        static Application& Get();

        Renderer* GetRenderer() const { return m_Renderer; }
        RendererAPI GetRendererType() const { return m_RendererType; }
        GLFWwindow* GetWindow() const { return m_Window; }
        Camera& GetMainCamera() { return m_MainCamera; }

        std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return m_GameObjects; }

    private:
        Camera m_MainCamera{};
        float m_Fov{ 0.0f };

        bool m_IsInitialized{false};
        bool m_StopRendering{false};

        int32_t m_FrameCount{0};

        GLFWwindow* m_Window = nullptr;

        Renderer* m_Renderer = nullptr;
        RendererAPI m_RendererType{RendererAPI::Vulkan};

        PhysicsWorld m_PhysicsWorld{};

        float m_Time{0.0f};
        float m_DeltaTime{0.0f};
        float m_LastFrameTime{0.0f};

        std::vector<std::shared_ptr<GameObject>> m_GameObjects;

    };
} // namespace Zero
