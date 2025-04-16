#pragma once

#include <GLFW/glfw3.h>
#include <Renderer/Vulkan/vk_types.h>

#include "camera/EditorCamera.h"
#include "Camera/PerspectiveCamera.h"
#include "renderer/OpenGLRenderer.h"
#include "renderer/VulkanRenderer.h"
#include <Scene/Scene.h>

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
        void Init();
        void Cleanup() const;
        void Run();

        void Draw();
        void UpdateImGui();
        
        void InitGLFW(RendererAPI rendererType);

        static float GetRandomFloat(float min, float max);

        void InitGameObjects();

        void SpawnSphere();
        void SpawnSphereAtLocation(const glm::vec3& location, float scale);

        bool IsEditorMode() const { return m_EditorMode; }

        static Application& Get();

        Renderer* GetRenderer() const { return m_Renderer; }
        RendererAPI GetRendererType() const { return m_RendererType; }
        GLFWwindow* GetWindow() const { return m_Window; }
        Camera& GetActiveCamera() const { return *m_ActiveCamera; }

    private:
        Camera* m_ActiveCamera = nullptr;
        EditorCamera m_EditorCamera{};
        PerspectiveCamera m_PlayerCamera{};
        float m_Fov{ 0.0f };

        bool m_EditorMode{ true };
        
        bool m_IsInitialized{false};
        bool m_StopRendering{false};

        int32_t m_FrameCount{0};

        GLFWwindow* m_Window = nullptr;

        Renderer* m_Renderer = nullptr;
        RendererAPI m_RendererType{ RendererAPI::Vulkan };

        std::shared_ptr<Scene> m_Scene{};
        // PhysicsWorld m_PhysicsWorld{};

        glm::vec4 m_DirectionalLightColor{1.0f, 1.0f, 1.0f, 0.2f};
        glm::vec3 m_DirectionalLightDirection{ 1.0f, 1.0, 0.0f };
        float m_DirectionalLightIntensity{1.f};

        float m_Time{0.0f};
        float m_DeltaTime{0.0f};
        float m_LastFrameTime{0.0f};
    };
} // namespace Zero
