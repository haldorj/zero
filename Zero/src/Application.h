#pragma once

#include <GLFW/glfw3.h>
#include <Renderer/Vulkan/vk_types.h>

#include "camera/EditorCamera.h"
#include "Camera/PerspectiveCamera.h"
#include "renderer/OpenGLRenderer.h"
#include "renderer/VulkanRenderer.h"
#include <Scene/Scene.h>

#include <Animation/Animator.h>

#include <Scene/Skybox/OpenGLSkybox.h>
#include <Scene/Skybox/VulkanSkybox.h>

namespace Zero
{
    enum class RendererAPI : uint8_t
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

	class SkyboxFactory
	{
	public:
		static Skybox* CreateSkybox(const RendererAPI type)
		{
			switch (type)
			{
			case RendererAPI::OpenGL: return new OpenGLSkybox();
			case RendererAPI::Vulkan: return new VulkanSkybox();
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
        bool ShowShadowmap() const { return m_ShowShadowmap; }

        static Application& Get();

        Renderer* GetRenderer() const { return m_Renderer; }
        RendererAPI GetRendererType() const { return m_RendererType; }
        GLFWwindow* GetWindow() const { return m_Window; }
        Camera& GetActiveCamera() const { return *m_ActiveCamera; }

    private:
        PerspectiveCamera m_PlayerCamera{};
        EditorCamera m_EditorCamera{};

        PhysicsWorld m_PhysicsWorld{};

        Camera* m_ActiveCamera = nullptr;
        GLFWwindow* m_Window = nullptr;
        Renderer* m_Renderer = nullptr;

        std::shared_ptr<Scene> m_Scene{};

        glm::vec4 m_DirectionalLightColor{0.4f, 0.7f, 1.0f, 0.3f};
        glm::vec3 m_DirectionalLightDirection{ 0.0f, 1.0, -1.0f };

        int32_t m_FrameCount{ 0 };

        float m_DirectionalLightIntensity{0.3f};
        float m_Time{0.0f};
        float m_DeltaTime{0.0f};
        float m_LastFrameTime{0.0f};

        float m_Fov{ 0.0f };

        RendererAPI m_RendererType{ RendererAPI::OpenGL };

        bool m_EditorMode{ true };
        bool m_IsInitialized{ false };
        bool m_StopRendering{ false };

        bool m_ShowShadowmap{ false };

    };
} // namespace Zero
