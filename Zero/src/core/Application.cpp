#include "Application.h"
#include "core/core.h"

#include <Renderer/Vulkan/vk_types.h>

#include "VkBootstrap.h"

#include <chrono>
#include <thread>
#include <Scene/GameObject.h>

#include <imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <random>



namespace Zero {
    // Choose RendererAPI
    static RendererAPI RendererType = RendererAPI::Vulkan;

    Application* LoadedEngine = nullptr;
    Application& Application::Get() { return *LoadedEngine; }

    void Application::InitGameObjects()
    {
        m_MainCamera.SetPosition({ 10, 10, -5 });
        FOV = m_MainCamera.GetFOV();

        std::array<std::string, 3> modelPaths{
            "../assets/models/black_bison2.fbx",
            "../assets/models/green_rhino2.fbx",
            "../assets/models/plane.glb",
        };

        m_GameObjects.reserve(modelPaths.size());

        std::shared_ptr<GameObject> BlackBison = std::make_shared<GameObject>(GameObject::Create());
        BlackBison->SetModel(ModelFactory::CreateModel(modelPaths[0].c_str(), RendererType));
        BlackBison->GetTransform().Location = { 0, 0, 0 };
        BlackBison->GetTransform().Rotation = { 0, 90, 0 };
        BlackBison->GetTransform().Scale = glm::vec3{ 0.5f };

        std::shared_ptr<GameObject> GreenRhino = std::make_shared<GameObject>(GameObject::Create());
        GreenRhino->SetModel(ModelFactory::CreateModel(modelPaths[1].c_str(), RendererType));
        GreenRhino->GetTransform().Location = { -15, 0, 0 };
        GreenRhino->GetTransform().Scale = glm::vec3{ 0.5f };
        GreenRhino->GetDynamics().Mass = 2;
        // GreenRhino->EnablePhysics = true;

        std::shared_ptr<GameObject> Plane = std::make_shared<GameObject>(GameObject::Create());
        Plane->SetModel(ModelFactory::CreateModel(modelPaths[2].c_str(), RendererType));
        Plane->GetTransform().Location = { 0, -2, 0 };
        Plane->GetTransform().Scale = glm::vec3{ 500.f };

        m_GameObjects.push_back(BlackBison);
        m_GameObjects.push_back(GreenRhino);
        m_GameObjects.push_back(Plane);

        glm::vec3 Direction = { 0, 1, -0.5 };
        Direction = glm::normalize(Direction);

        float Force = 20;
        glm::vec3 ForceVector = Direction * Force;

        m_GameObjects[1]->GetDynamics().AddImpulse(ForceVector);
    }

    void Application::SpawnSphere()
    {
        float x = GetRandomFloat(0.5, 3);

        std::shared_ptr<GameObject> Sphere = std::make_shared<GameObject>(GameObject::Create());
		Sphere->SetModel(ModelFactory::CreateModel("../assets/models/sphere.glb", RendererType));
		Sphere->GetTransform().Location = m_MainCamera.GetPosition();
		Sphere->GetTransform().Scale = glm::vec3{ x };
		Sphere->GetDynamics().Mass = x;
		Sphere->EnablePhysics = true;

        glm::vec3 Direction = m_MainCamera.GetForwardVector();
        Sphere->GetDynamics().AddImpulse(Direction * 50.0f);

		m_GameObjects.push_back(Sphere);
    }

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

        // Mesh* Rectangle = new Mesh(rectVertices, rectIndices);

        // m_Renderer->InitObject();
    }

    void Application::Init()
    {
        // Only one engine initialization is allowed with the application.
        assert(LoadedEngine == nullptr);
        LoadedEngine = this;

        InitGLFW(RendererType);

        // Initialize the renderer
        m_Renderer = RendererFactory::CreateRenderer(RendererType);
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

    bool pressed = false;
    void Application::Run()
    {
        // main loop
        while (!glfwWindowShouldClose(m_Window))
        {
            // Spawn Sphere
            if (glfwGetKey(m_Window, GLFW_KEY_Q) == GLFW_PRESS)
			{
                if (!pressed)
                {
                    SpawnSphere();
                    pressed = true;
                }
			}
			else
			{
				pressed = false;
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

            for (auto& gameObject : m_GameObjects)
            {
                gameObject->Update(m_DeltaTime);
            }

            m_GameObjects[0].get()->GetTransform().Rotation.y += m_DeltaTime * 1;
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

        switch (RendererType)
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
        ImGui::SliderFloat("Camera FOV: ", &FOV, 1.0f, 120.0f);
        m_MainCamera.SetFOV(FOV);
        //ImGui::Checkbox("Test Checkbox", &TestBool);
        ImGui::End();

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

    float  Application::GetRandomFloat(float min, float max) {
        // Create a random device and a Mersenne Twister engine
        std::random_device rd;
        std::mt19937 gen(rd());

        // Define the distribution range
        std::uniform_real_distribution<float> dis(min, max);

        // Generate and return the random float
        return dis(gen);
    }
}
