#include "Application.h"

#include <chrono>
#include <imgui.h>
#include <random>
#include <thread>
#include <Renderer/Vulkan/vk_types.h>
#include <Scene/GameObject.h>

#include "VkBootstrap.h"
#include "core/core.h"
#include "Physics/Collision/CapsuleCollider.h"
#include "Physics/Collision/PlaneCollider.h"
#include "Physics/Collision/SphereCollider.h"
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Zero
{
    Application* Application::s_Instance = nullptr;

    void Application::InitGameObjects()
    {
        m_EditorCamera.SetPosition({10, 10, -5});
        m_Fov = m_EditorCamera.GetFOV();

        m_ActiveCamera = m_EditorMode
                             ? reinterpret_cast<Camera*>(&m_EditorCamera)
                             : reinterpret_cast<Camera*>(&m_PlayerCamera);

        if (!m_ActiveCamera)
        {
            throw std::runtime_error("Active camera is not set");
        }

        const std::array<std::string, 3> modelPaths{
            "assets/models/Prototyping 1.1/GLTF/dummy_platformer.gltf",
            "assets/models/green_rhino2.fbx",
            "assets/models/plane.glb",
        };

        std::shared_ptr<GameObject> player = std::make_shared<GameObject>(GameObject::Create());
        player->SetModel(ModelFactory::CreateModel(modelPaths[0].c_str(), m_RendererType));
        player->GetTransform().Position = {15, 10, 0};
        player->GetTransform().Scale = glm::vec3{1.0f};
        player->GetRigidBody().Mass = 5;
        player->SetCollider(std::make_shared<CapsuleCollider>(glm::vec3{0, 1.2, 0}, 0.5f, 1.5f));
        player->EnableGravity = true;
        player->EnableCollision = true;

        player->GetAnimator() = std::make_shared<Animator>();
		player->GetAnimator()->LoadAnimations(modelPaths[0], player->GetModel().get());
        player->SetAnimation(2);

        std::shared_ptr<GameObject> greenRhino = std::make_shared<GameObject>(GameObject::Create());
        greenRhino->SetModel(ModelFactory::CreateModel(modelPaths[1].c_str(), m_RendererType));
        greenRhino->GetTransform().Position = {-15, 0, 0};
        greenRhino->GetTransform().Scale = glm::vec3{0.5f};
        greenRhino->GetRigidBody().Mass = 2;

        std::shared_ptr<GameObject> plane = std::make_shared<GameObject>(GameObject::Create());
        plane->SetModel(ModelFactory::CreateModel(modelPaths[2].c_str(), m_RendererType));
        plane->GetTransform().Position = {0, 0, 0};
        plane->GetTransform().Scale = glm::vec3{50.f};
        plane->SetCollider(std::make_shared<PlaneCollider>(plane->GetTransform().GetUpVector(), 500.f));
        plane->EnableGravity = false;
        plane->EnableCollision = true;

        DirectionalLight* dirLight = new DirectionalLight({
            glm::vec3(m_DirectionalLightColor),
            m_DirectionalLightColor.w,
            m_DirectionalLightDirection,
            m_DirectionalLightIntensity 
            });

        m_Scene = std::make_shared<Scene>(dirLight);

        m_Scene->AddGameObject(player);
        m_Scene->AddGameObject(greenRhino);
        m_Scene->AddGameObject(plane);

        m_Scene->AddPointLight(std::make_shared<PointLight>(
            glm::vec3{ 10, 10, -10 },
            glm::vec3{ 1, 0, 0.5 },
            0.1f, 0.25f,
            1.0, 0.014, 0.007));
        m_Scene->AddPointLight(std::make_shared<PointLight>(
            glm::vec3{ 0, 15, 10 },
            glm::vec3{ 0.5, 1.0, 0.0 },
            0.1f, 0.25f,
            1.0, 0.014, 0.007));
        m_Scene->AddPointLight(std::make_shared<PointLight>(
            glm::vec3{ -10, 10, -10 },
            glm::vec3{ 0, 0, 1 },
            0.1f, 0.25f,
            1.0, 0.014, 0.007));

        m_Scene->AddSpotLight(std::make_shared<SpotLight>(
			glm::vec3{ 0, 30, 0 },
			glm::vec3{ 1, 1, 0 },
			0.2f, 0.5f,
			1.0, 0.014, 0.007,
			glm::vec3{ 0, -1, 0 }, 35.0f));

        SpawnSphereAtLocation({0, 0, 20}, 10.0f);

        glm::vec3 direction = {0, 1, -0.5};
        direction = glm::normalize(direction);

        constexpr float force = 20;
        const glm::vec3 forceVector = direction * force;

        // m_GameObjects[1]->GetRigidBody().AddImpulse(forceVector);
    }

    void Application::SpawnSphere()
    {
        const float x = GetRandomFloat(0.5, 3);

        const auto sphere = std::make_shared<GameObject>(GameObject::Create());
        sphere->SetModel(ModelFactory::CreateModel("assets/models/sphere.glb", m_RendererType));
        sphere->GetTransform().Position = m_EditorCamera.GetPosition();
        sphere->GetTransform().Scale = glm::vec3{x};
        sphere->GetRigidBody().Mass = x;
        sphere->SetCollider(std::make_shared<SphereCollider>(glm::vec3{0, 0, 0}, x));
        sphere->EnableGravity = true;
        sphere->EnableCollision = true;

        const glm::vec3 direction = m_EditorCamera.GetForwardVector();
        sphere->GetRigidBody().AddImpulse(direction * 50.0f);

        m_Scene->AddGameObject(sphere);
        // m_GameObjects.emplace_back(sphere);
    }

    void Application::SpawnSphereAtLocation(const glm::vec3& location, float scale)
    {
        const auto sphere = std::make_shared<GameObject>(GameObject::Create());
        sphere->SetModel(ModelFactory::CreateModel("assets/models/sphere.glb", m_RendererType));
        sphere->GetTransform().Position = location;
        sphere->GetTransform().Scale = glm::vec3{scale};
        sphere->GetRigidBody().Mass = scale;
        sphere->SetCollider(std::make_shared<SphereCollider>(glm::vec3{0, 0, 0}, scale));
        sphere->EnableGravity = false;
        sphere->EnableCollision = true;

        const glm::vec3 direction = m_EditorCamera.GetForwardVector();
        sphere->GetRigidBody().AddImpulse(direction * 50.0f);

        m_Scene->AddGameObject(sphere);
    }

    void Application::Init()
    {
        // Only one engine initialization is allowed with the application.
        assert(s_Instance == nullptr);
        s_Instance = this;

        InitGLFW(m_RendererType);

        m_PhysicsWorld.Init();

        // Initialize the renderer
        m_Renderer = RendererFactory::CreateRenderer(m_RendererType);
        m_Renderer->Init();
        m_Renderer->InitImGui();

        InitGameObjects();

        m_Scene->SetSkybox(SkyboxFactory::CreateSkybox(m_RendererType));
        m_Scene->GetSkybox()->LoadCubeMap({
            "assets/skybox/sb_strato/stratosphere_rt.tga",
            "assets/skybox/sb_strato/stratosphere_lf.tga",
            "assets/skybox/sb_strato/stratosphere_up.tga",
            "assets/skybox/sb_strato/stratosphere_dn.tga",
            "assets/skybox/sb_strato/stratosphere_bk.tga",
            "assets/skybox/sb_strato/stratosphere_ft.tga"
            });
    }

    void Application::Cleanup() const
    {
        m_Scene->Destroy();
        m_Renderer->Shutdown();

        glfwDestroyWindow(m_Window);
        glfwTerminate();

        s_Instance = nullptr;
    }

    bool Pressed = false;
    bool Loaded = false;

    void Application::Run()
    {
        // main loop
        while (!glfwWindowShouldClose(m_Window))
        {
            // Spawn Sphere
            if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
            {
                if (!Pressed)
                {
                    SpawnSphere();
                    Pressed = true;
                }
            }
            else
            {
                Pressed = false;
            }

            m_Time = static_cast<float>(glfwGetTime());
            m_DeltaTime = m_Time - m_LastFrameTime;
            m_LastFrameTime = m_Time;

            // Poll and handle events
            glfwPollEvents();

            // Check if the window is minimized
            if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            
            
            m_ActiveCamera = m_EditorMode
                                 ? reinterpret_cast<Camera*>(&m_EditorCamera)
                                 : reinterpret_cast<Camera*>(&m_PlayerCamera);

            if (m_EditorMode)
            {
                m_EditorCamera.ProcessInput(m_Window, m_DeltaTime);
                m_EditorCamera.Update(m_DeltaTime);
            }
            else
            {
                
                m_PlayerCamera.ProcessInput(m_Window, m_DeltaTime);
                m_PlayerCamera.Update(m_DeltaTime, m_Scene->GetGameObjects()[0]->GetTransform().Position);
            }
            
            if (Loaded)
            {
                m_Scene->GetGameObjects()[0]->UpdatePlayer(m_DeltaTime);
                m_PhysicsWorld.Step(m_DeltaTime, m_Scene->GetGameObjects());
            }
            
            m_Scene->GetDirectionalLight()->Update(
                glm::vec3(m_DirectionalLightColor), m_DirectionalLightColor.a,
                m_DirectionalLightDirection, m_DirectionalLightIntensity);

            for (const auto& gameObject : m_Scene->GetGameObjects())
            {
                gameObject->UpdateAnimation(m_DeltaTime);
            }

            Draw();
            
            if (!Loaded)
                Loaded = true;
        }
    }

    void Application::Draw()
    {
        m_Renderer->SetClearColor({0.05, 0, 0.32, 1});
        m_Renderer->Draw(m_Scene.get());

        m_FrameCount++;
    }

    void Application::UpdateImGui()
    {
        ImGui::Begin("Zero");

        switch (m_RendererType)
        {
        case RendererAPI::OpenGL:
            ImGui::Text("OpenGL");
            break;
        case RendererAPI::Vulkan:
            ImGui::Text("Vulkan");
            break;
        default:
            break;
        }
        ImGui::Text("FPS: %i", static_cast<int>(1.0f / m_DeltaTime));
        //ImGui::Checkbox("VSync ", &m_Renderer->VSync);
        ImGui::Checkbox("Editor Mode", &m_EditorMode);
        ImGui::Checkbox("Show Shadow Map", &m_ShowShadowMap);
        ImGui::End();

        ImGui::Begin("Camera");
        ImGui::Text("Camera Position: { %.2f, %.2f, %.2f }",
                    m_EditorCamera.GetPosition().x, m_EditorCamera.GetPosition().y, m_EditorCamera.GetPosition().z);
        ImGui::Text("Camera Direction: { %.2f, %.2f, %.2f }",
                    m_EditorCamera.GetDirection().x, m_EditorCamera.GetDirection().y, m_EditorCamera.GetDirection().z);
        ImGui::SliderFloat("Camera FOV: ", &m_Fov, 1.0f, 120.0f);
        m_EditorCamera.SetFOV(m_Fov);
        m_PlayerCamera.SetFOV(m_Fov);
        ImGui::End();

        ImGui::Begin("Objects");
        ImGui::Text("Number of Objects: %i", static_cast<int>(m_Scene->GetGameObjects().size()));
        ImGui::End();

        ImGui::Begin("Directional Light");
        ImGui::ColorEdit4("Color:", glm::value_ptr(m_DirectionalLightColor));
        ImGui::DragFloat3("Direction", glm::value_ptr(m_DirectionalLightDirection), 0.1f);
        ImGui::DragFloat("Diffuse Intensity", &m_DirectionalLightIntensity, 0.05f, 0.0f, 1.0f);
        ImGui::End();
    }

    void Application::InitGLFW(const RendererAPI rendererType)
    {
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
            // OpenGL v. 4.6
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            // CORE PROFILE: modern OpenGL
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
        m_Window = glfwCreateWindow(EXTENT_WIDTH, EXTENT_HEIGHT, "ZeroEngine", nullptr, nullptr);
        if (!m_Window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }
    }

    float Application::GetRandomFloat(const float min, const float max)
    {
        // Create a random device and a Mersenne Twister engine
        std::random_device rd;
        std::mt19937 gen(rd());

        // Define the distribution range
        std::uniform_real_distribution dis(min, max);

        // Generate and return the random float
        return dis(gen);
    }
}
