#include "OpenGLRenderer.h"
#include <Application.h>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

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

        glm::mat4 projection = glm::perspective(glm::radians(Application::Get().GetActiveCamera().GetFOV()),
            (float)m_Width / (float)m_Height, 0.1f, 10000.0f);

        scene->GetSkybox()->Draw(projection, Application::Get().GetActiveCamera().GetViewMatrix());

        glm::mat4 model = glm::mat4(1.0f);

		m_ShaderProgram->Activate();
        SetUniformValues(m_ShaderProgram.get(), scene);

        for (auto& gameObj : scene->GetGameObjects())
        {
            if (!gameObj->GetModel())
            {
                continue;
            }

            if (gameObj->GetAnimator())
            {
                glUniform1i(glGetUniformLocation(m_ShaderProgram->GetID(), "Animated"), 1);

                const std::vector transforms = gameObj->GetAnimator()->GetFinalBoneMatrices();
                for (int i = 0; i < transforms.size(); ++i)
                {
                    std::string str = "finalBonesMatrices[" + std::to_string(i) + "]";

                    int transformLoc = glGetUniformLocation(m_ShaderProgram->GetID(), str.c_str());
                    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transforms[i]));
                }
            }
            else
            {
                glUniform1i(glGetUniformLocation(m_ShaderProgram->GetID(), "Animated"), 0);
            }

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
        int pointLightCount = static_cast<int>(scene->GetPointLights().size());
        m_PointLightCount = static_cast<int>(scene->GetPointLights().size());
        glUniform1i(glGetUniformLocation(shader->GetID(), "pointLightCount"), static_cast<GLint>(m_PointLightCount));

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

        // Spot Lights
        int spotLightCount = static_cast<int>(scene->GetSpotLights().size());
        m_SpotLightCount = scene->GetSpotLights().size();
        glUniform1i(glGetUniformLocation(shader->GetID(), "spotLightCount"), static_cast<GLint>(m_SpotLightCount));

        for (unsigned int i = 0; i < scene->GetSpotLights().size(); ++i)
        {
			char locBuff[100] = { '\0' };

			snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.color", i);
			m_UniformSpotLights[i].Color = glGetUniformLocation(shader->GetID(), locBuff);
			glUniform3fv(m_UniformSpotLights[i].Color, 1, glm::value_ptr(scene->GetSpotLights()[i]->GetColor()));

			snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.ambientIntensity", i);
			m_UniformSpotLights[i].AmbientIntensity = glGetUniformLocation(shader->GetID(), locBuff);
			glUniform1f(m_UniformSpotLights[i].AmbientIntensity, scene->GetSpotLights()[i]->GetAmbientIntensity());

			snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.base.diffuseIntensity", i);
			m_UniformSpotLights[i].DiffuseIntensity = glGetUniformLocation(shader->GetID(), locBuff);
			glUniform1f(m_UniformSpotLights[i].DiffuseIntensity, scene->GetSpotLights()[i]->GetDiffuseIntensity());

			snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.position", i);
			m_UniformSpotLights[i].Position = glGetUniformLocation(shader->GetID(), locBuff);
			glUniform3fv(m_UniformSpotLights[i].Position, 1, glm::value_ptr(scene->GetSpotLights()[i]->GetPosition()));

			snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.constant", i);
			m_UniformSpotLights[i].Constant = glGetUniformLocation(shader->GetID(), locBuff);
			glUniform1f(m_UniformSpotLights[i].Constant, scene->GetSpotLights()[i]->GetConstant());

			snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.linear", i);
			m_UniformSpotLights[i].Linear = glGetUniformLocation(shader->GetID(), locBuff);
			glUniform1f(m_UniformSpotLights[i].Linear, scene->GetSpotLights()[i]->GetLinear());

			snprintf(locBuff, sizeof(locBuff), "spotLights[%d].base.exponent", i);
            m_UniformSpotLights[i].Exponent = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform1f(m_UniformSpotLights[i].Exponent, scene->GetSpotLights()[i]->GetExponent());

            snprintf(locBuff, sizeof(locBuff), "spotLights[%d].direction", i);
            m_UniformSpotLights[i].Direction = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform3fv(m_UniformSpotLights[i].Direction, 1, glm::value_ptr(scene->GetSpotLights()[i]->GetDirection()));

            snprintf(locBuff, sizeof(locBuff), "spotLights[%d].edge", i);
            m_UniformSpotLights[i].Edge = glGetUniformLocation(shader->GetID(), locBuff);
            glUniform1f(m_UniformSpotLights[i].Edge, scene->GetSpotLights()[i]->GetEdge());
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
