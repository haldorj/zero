#include "OpenGLRenderer.h"
#include <Application.h>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include "Debug/DebugSphere.h"

std::vector<Vertex> RectVertices;
std::vector<GLuint> RectIndeces;

namespace Zero
{
    // Callback function to handle window resizing
    void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        // Adjusts the viewport to the new window dimensions
        glViewport(0, 0, width, height);
    }

    void OpenGLRenderer::Init()
    {
        std::cout << "ZeroEngine OpenGL \n";

        glfwMakeContextCurrent(Application::Get().GetWindow());

        glfwSetFramebufferSizeCallback(Application::Get().GetWindow(), FramebufferSizeCallback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cout << "Failed to initialize GLAD" << '\n';
            return;
        }

        m_Width = EXTENT_WIDTH;
        m_Height = EXTENT_HEIGHT;

        glfwGetFramebufferSize(Application::Get().GetWindow(), &m_Width, &m_Height);
        glViewport(0, 0, m_Width, m_Height);

        glfwSwapInterval(1); // vsync

        const GLubyte* renderer = glGetString(GL_RENDERER);
        std::cout << "Chosen GPU: " << "\n";
        std::cout << "\t" << renderer << "\n";

        InitShaders();

        // Enable depth testing for 3D
        glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRenderer::InitImGui()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(Application::Get().GetWindow(), true);
        // GLSL ver. 450
        ImGui_ImplOpenGL3_Init("#version 450");
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
    }


    void OpenGLRenderer::Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_ShaderProgram)
            m_ShaderProgram->Delete();
    }

    void OpenGLRenderer::Draw(Scene* scene)
    {
        // Specify the color of the background
        glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
        // Clean the back buffer and assign the new color to it
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        glfwGetWindowSize(Application::Get().GetWindow(), &m_Width, &m_Height);

        // model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        view = Application::Get().GetActiveCamera().GetViewMatrix();
        projection = glm::perspective(glm::radians(Application::Get().GetActiveCamera().GetFOV()),
                                      (float)m_Width / (float)m_Height, 0.1f, 10000.0f);

        int viewLoc = glGetUniformLocation(m_ShaderProgram->GetID(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projLoc = glGetUniformLocation(m_ShaderProgram->GetID(), "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        int viewPos = glGetUniformLocation(m_ShaderProgram->GetID(), "viewPos");
        glUniform3fv(viewPos, 1, glm::value_ptr(Application::Get().GetActiveCamera().GetPosition()));

        int ambientColor = glGetUniformLocation(m_ShaderProgram->GetID(), "directionalLight.color");
        glUniform3fv(ambientColor, 1, glm::value_ptr(scene->GetDirectionalLight()->GetColor()));

        int ambientIntensity = glGetUniformLocation(m_ShaderProgram->GetID(), "directionalLight.ambientIntensity");
        glUniform1f(ambientIntensity, scene->GetDirectionalLight()->GetAmbientIntensity());

        for (auto& gameObj : scene->GetGameObjects())
        {
            if (!gameObj->GetModel())
                continue;

            model = gameObj->GetTransform().GetMatrix();

            gameObj->GetModel()->Draw(*m_ShaderProgram, model);
        }

        Application::Get().UpdateImGui();

        //make imgui calculate internal draw structures
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the back buffer with the front buffer
        glfwSwapBuffers(Application::Get().GetWindow());
    }

    void OpenGLRenderer::InitShaders()
    {
        // Create Shader object
        m_ShaderProgram = std::make_unique<OpenGLShader>(
            "../shaders/OpenGL/default.vert", 
            "../shaders/OpenGL/default.frag"
        );
    }


}
