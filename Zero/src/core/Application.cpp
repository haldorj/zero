#include "Application.h"

#include <chrono>
#include <imgui.h>
#include <random>
#include <thread>
#include <Renderer/Vulkan/vk_types.h>
#include <Scene/GameObject.h>

#include "VkBootstrap.h"
#include "core/core.h"


namespace Zero {
    Application* LoadedEngine = nullptr;
    Application& Application::Get() { return *LoadedEngine; }

    void Application::InitGameObjects()
    {
        m_MainCamera.SetPosition({ 10, 10, -5 });
        m_Fov = m_MainCamera.GetFOV();

        const std::array<std::string, 3> modelPaths{
            "../assets/models/black_bison2.fbx",
            "../assets/models/green_rhino2.fbx",
            "../assets/models/plane.glb",
        };

        m_GameObjects.reserve(modelPaths.size());

        std::shared_ptr<GameObject> blackBison = std::make_shared<GameObject>(GameObject::Create());
        blackBison->SetModel(ModelFactory::CreateModel(modelPaths[0].c_str(), m_RendererType));
        blackBison->GetTransform().Location = { 15, 0, 0 };
        blackBison->GetTransform().Scale = glm::vec3{ 0.5f };
        blackBison->SetCollider(std::make_shared<SphereCollider>(glm::vec3{ 0, 0, 0 }, 5.0f));
        blackBison->GetDynamics().Mass = 100;
        blackBison->EnableCollision = true;

        std::shared_ptr<GameObject> greenRhino = std::make_shared<GameObject>(GameObject::Create());
        greenRhino->SetModel(ModelFactory::CreateModel(modelPaths[1].c_str(), m_RendererType));
        greenRhino->GetTransform().Location = { -15, 0, 0 };
        greenRhino->GetTransform().Scale = glm::vec3{ 0.5f };
        greenRhino->GetDynamics().Mass = 2;

        std::shared_ptr<GameObject> plane = std::make_shared<GameObject>(GameObject::Create());
        plane->SetModel(ModelFactory::CreateModel(modelPaths[2].c_str(), m_RendererType));
        plane->GetTransform().Location = { 0, -2, 0 };
        plane->GetTransform().Scale = glm::vec3{ 500.f };
        plane->SetCollider(std::make_shared<PlaneCollider>(plane->GetTransform().GetUpVector(), 500.f));
        plane->EnableGravity = false;
        plane->EnableCollision = true;

        m_GameObjects.emplace_back(blackBison);
        m_GameObjects.emplace_back(greenRhino);
        m_GameObjects.emplace_back(plane);

        glm::vec3 direction = { 0, 1, -0.5 };
        direction = glm::normalize(direction);

        constexpr float force = 20;
        const glm::vec3 forceVector = direction * force;

        m_GameObjects[1]->GetDynamics().AddImpulse(forceVector);
    }

    void Application::SpawnSphere()
    {
        const float x = GetRandomFloat(0.5, 3);

        const auto sphere = std::make_shared<GameObject>(GameObject::Create());
		sphere->SetModel(ModelFactory::CreateModel("../assets/models/sphere.glb", m_RendererType));
		sphere->GetTransform().Location = m_MainCamera.GetPosition();
		sphere->GetTransform().Scale = glm::vec3{ x };
		sphere->GetDynamics().Mass = x;
        sphere->SetCollider(std::make_shared<SphereCollider>(glm::vec3 {0, 0, 0}, x));
		sphere->EnableGravity = true;
        sphere->EnableCollision = true;

        const glm::vec3 direction = m_MainCamera.GetForwardVector();
        sphere->GetDynamics().AddImpulse(direction * 50.0f);

		m_GameObjects.emplace_back(sphere);
    }

    void Application::DestroyGameObject(GameObject::IdType objectID)
    {
        for (auto it = m_GameObjects.begin(); it != m_GameObjects.end(); ++it)
        {
            if ((*it)->GetID() == objectID)
            {
                (*it)->Destroy();
                m_GameObjects.erase(it);
                
                break;
            }
        }
    }

    void Application::Init()
    {
        // Only one engine initialization is allowed with the application.
        assert(LoadedEngine == nullptr);
        LoadedEngine = this;

        InitGLFW(m_RendererType);

        m_PhysicsWorld.Init();
        
        // Initialize the renderer
        m_Renderer = RendererFactory::CreateRenderer(m_RendererType);
        m_Renderer->Init();
        m_Renderer->InitImGui();

        InitGameObjects();

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

    bool Pressed = false;
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
            
            m_MainCamera.ProcessInput(m_Window, m_DeltaTime);
            m_MainCamera.Update(m_DeltaTime);

            m_GameObjects[0]->GetTransform().Rotation.y += m_DeltaTime * 1;
            m_PhysicsWorld.Step(m_DeltaTime, m_GameObjects);
            
            Draw();
        }
    }

    void Application::Draw()
    {
        m_Renderer->SetClearColor({0.05, 0, 0.32, 1});
        m_Renderer->Draw(m_GameObjects, Topology::Triangles);

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
        ImGui::End();

        ImGui::Begin("Camera");
        ImGui::Text("Camera Position: { %.2f, %.2f, %.2f }", m_MainCamera.GetPosition().x, m_MainCamera.GetPosition().y, m_MainCamera.GetPosition().z);
        ImGui::Text("Camera Direction: { %.2f, %.2f, %.2f }", m_MainCamera.GetDirection().x, m_MainCamera.GetDirection().y, m_MainCamera.GetDirection().z);
        ImGui::SliderFloat("Camera FOV: ", &m_Fov, 1.0f, 120.0f);
        m_MainCamera.SetFOV(m_Fov);
        ImGui::End();

        ImGui::Begin("Objects");
        ImGui::Text("Number of Objects: %i", m_GameObjects.size());
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

    float  Application::GetRandomFloat(const float min, const float max) {
        // Create a random device and a Mersenne Twister engine
        std::random_device rd;
        std::mt19937 gen(rd());

        // Define the distribution range
        std::uniform_real_distribution dis(min, max);

        // Generate and return the random float
        return dis(gen);
    }
}
