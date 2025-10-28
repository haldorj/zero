#pragma once

#include <GLFW/glfw3.h>
#include <Renderer/Vulkan/vk_types.h>

#include "camera/EditorCamera.h"
#include "Camera/PerspectiveCamera.h"

#include <Scene/Scene.h>

#include <Animation/Animator.h>

namespace Zero
{
    class Renderer;
    class Skybox;

    enum class RendererAPI : uint8_t
    {
        OpenGL,
        Vulkan,
    };

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

        void InitScene();

        void SpawnSphere();
        void SpawnSphereAtLocation(const glm::vec3& location, float scale);

        bool IsEditorMode() const { return m_EditorMode; }
        bool ShowShadowMap() const { return m_ShowShadowMap; }

        inline static Application& Get() { return *s_Instance; }

        Renderer* GetRenderer() const { return m_Renderer; }
        GLFWwindow* GetWindow() const { return m_Window; }
        RendererAPI GetRendererType() const { return m_RendererType; }
        Camera& GetActiveCamera() const { return *m_ActiveCamera; }

    private:
        PerspectiveCamera m_PlayerCamera{};
        EditorCamera m_EditorCamera{};

        PhysicsWorld m_PhysicsWorld{};

        std::unique_ptr<Scene> m_Scene{};

		std::shared_ptr<Model> m_PlayerModel{};
		std::shared_ptr<Model> m_GreenRhinoModel{};
        std::shared_ptr<Model> m_PlaneModel{};
		std::shared_ptr<Model> m_SphereModel{};

        Camera* m_ActiveCamera = nullptr;
        GLFWwindow* m_Window = nullptr;
        Renderer* m_Renderer = nullptr;

        glm::vec4 m_DirectionalLightColor{0.4f, 0.7f, 1.0f, 0.3f};
        glm::vec3 m_DirectionalLightDirection{ 0.0f, 1.0, -1.0f };

        float m_DirectionalLightIntensity{0.3f};
        float m_Time{0.0f};
        float m_DeltaTime{0.0f};
        float m_LastFrameTime{0.0f};

        float m_Fov{ 0.0f };

        RendererAPI m_RendererType{ RendererAPI::OpenGL };

        bool m_EditorMode{ true };
        bool m_ShowShadowMap{ false };

        static Application* s_Instance;
    };
} // namespace Zero
