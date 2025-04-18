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

        glfwGetWindowSize(Application::Get().GetWindow(), &m_Width, &m_Height);

        glm::mat4 model = glm::mat4(1.0f);

        SetUniformValues(m_ShaderProgram.get(), scene);

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

    void OpenGLRenderer::SetUniformValues(OpenGLShader* shader, Scene* scene)
    {
        if (!shader) return;

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        view = Application::Get().GetActiveCamera().GetViewMatrix();
        projection = glm::perspective(glm::radians(Application::Get().GetActiveCamera().GetFOV()),
            (float)m_Width / (float)m_Height, 0.1f, 10000.0f);

        int viewLoc = glGetUniformLocation(shader->GetID(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projLoc = glGetUniformLocation(shader->GetID(), "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        int viewPos = glGetUniformLocation(shader->GetID(), "viewPos");
        glUniform3fv(viewPos, 1, glm::value_ptr(Application::Get().GetActiveCamera().GetPosition()));

        // Directional Light
        m_UniformDirectionalLight.Color = glGetUniformLocation(shader->GetID(), "directionalLight.base.color");
        glUniform3fv(m_UniformDirectionalLight.Color, 1, glm::value_ptr(scene->GetDirectionalLight()->GetColor()));

        m_UniformDirectionalLight.AmbientIntensity = glGetUniformLocation(shader->GetID(), "directionalLight.base.ambientIntensity");
        glUniform1f(m_UniformDirectionalLight.AmbientIntensity, scene->GetDirectionalLight()->GetAmbientIntensity());

        m_UniformDirectionalLight.DiffuseIntensity = glGetUniformLocation(shader->GetID(), "directionalLight.base.diffuseIntensity");
        glUniform1f(m_UniformDirectionalLight.DiffuseIntensity, scene->GetDirectionalLight()->GetDiffuseIntensity());

        m_UniformDirectionalLight.Direction = glGetUniformLocation(shader->GetID(), "directionalLight.direction");
        glUniform3fv(m_UniformDirectionalLight.Direction, 1, glm::value_ptr(scene->GetDirectionalLight()->GetDirection()));

        // Material
        int specularIntensity = glGetUniformLocation(shader->GetID(), "material.specularIntensity");
        glUniform1f(specularIntensity, scene->GetMaterial()->GetSpecularIntensity());

        int shininess = glGetUniformLocation(shader->GetID(), "material.shininess");
        glUniform1f(shininess, scene->GetMaterial()->GetShininess());

        // Point Lights
        int pointLightCount = scene->GetPointLights().size();
        m_PointLightCount = scene->GetPointLights().size();
        glUniform1i(glGetUniformLocation(shader->GetID(), "pointLightCount"), m_PointLightCount);

        for (unsigned int i = 0; i < scene->GetPointLights().size(); ++i)
        {
            char locBuff[100] = { '\0' };

            snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.color", i);
            m_UniformPointLights[i].Color = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform3fv(m_UniformPointLights[i].Color, 1, glm::value_ptr(scene->GetPointLights()[i]->GetColor()));

            snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.ambientIntensity", i);
            m_UniformPointLights[i].AmbientIntensity = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform1f(m_UniformPointLights[i].AmbientIntensity, scene->GetPointLights()[i]->GetAmbientIntensity());

            snprintf(locBuff, sizeof(locBuff), "pointLights[%d].base.diffuseIntensity", i);
            m_UniformPointLights[i].DiffuseIntensity = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform1f(m_UniformPointLights[i].DiffuseIntensity, scene->GetPointLights()[i]->GetDiffuseIntensity());

            snprintf(locBuff, sizeof(locBuff), "pointLights[%d].position", i);
            m_UniformPointLights[i].Position = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform3fv(m_UniformPointLights[i].Position, 1, glm::value_ptr(scene->GetPointLights()[i]->GetPosition()));

            snprintf(locBuff, sizeof(locBuff), "pointLights[%d].constant", i);
            m_UniformPointLights[i].Constant = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform1f(m_UniformPointLights[i].Constant, scene->GetPointLights()[i]->GetConstant());

            snprintf(locBuff, sizeof(locBuff), "pointLights[%d].linear", i);
            m_UniformPointLights[i].Linear = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform1f(m_UniformPointLights[i].Linear, scene->GetPointLights()[i]->GetLinear());

            snprintf(locBuff, sizeof(locBuff), "pointLights[%d].exponent", i);
            m_UniformPointLights[i].Exponent = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform1f(m_UniformPointLights[i].Exponent, scene->GetPointLights()[i]->GetExponent());
        }
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
