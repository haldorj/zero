#include "OpenGLRenderer.h"
#include <core/core.h>
#include <core/Application.h>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<Vertex> RectVertices;
std::vector<GLuint> RectIndeces;

namespace Zero
{
    void OpenGLRenderer::Init()
    {
        std::cout << "ZeroEngine OpenGL \n";

        glfwMakeContextCurrent(Application::Get().GetWindow());

        // Why c-style cast ?, reinterpret_cast?
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << '\n';
            return;
        }

        glViewport(0, 0, EXTENT_WIDTH, EXTENT_HEIGHT);

        glfwSwapInterval(1); // vsync on

        const GLubyte* renderer = glGetString(GL_RENDERER);
        std::cout << "Chosen GPU: " << "\n";
        std::cout << "\t" << renderer << "\n";

        InitShaders();

        // Enable depth testing for 3D
        glEnable(GL_DEPTH_TEST);
    }


    void OpenGLRenderer::InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices)
    {
        for (const auto& vertex : vertices)
        {
            RectVertices.push_back(vertex);
        }
        for (const auto& index : indices)
        {
            RectIndeces.push_back(static_cast<GLuint>(index));
        }

        std::string path = "../assets/images/brick.png";
        OpenGLTexture TEX(path.c_str(), "diffuse", 0, GL_RGBA, GL_UNSIGNED_BYTE);

        std::vector<OpenGLTexture> pyramidTextures;
        pyramidTextures.push_back(TEX);

        Pyramid = new Mesh(RectVertices, RectIndeces, pyramidTextures);
    }

    void OpenGLRenderer::Shutdown()
    {
        shaderProgram->Delete();

    }

    float rotation = 0.0f;
    double lastTime = glfwGetTime();

    void OpenGLRenderer::Draw(Topology topology)
    {
        // Specify the color of the background
        glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
        // Clean the back buffer and assign the new color to it
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Tell OpenGL which Shader Program we want to use
   /*     shaderProgram->Activate();*/

        double currentTime = glfwGetTime();

        if (currentTime - lastTime >= 1 / 60)
        {
            rotation += 0.5f;
            lastTime = currentTime;
        }

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        view = Application::Get().GetMainCamera().GetViewMatrix();
        projection = glm::perspective(glm::radians(70.0f), (float)EXTENT_WIDTH / (float)EXTENT_HEIGHT, 0.1f, 100.0f);

        int modelLoc = glGetUniformLocation(shaderProgram->GetID(), "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        int viewLoc = glGetUniformLocation(shaderProgram->GetID(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projLoc = glGetUniformLocation(shaderProgram->GetID(), "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        Pyramid->Draw(*shaderProgram, Application::Get().GetMainCamera());

       
        // Draw primitives, number of indices, datatype of indices, index of indices
        // 
        //switch (topology)
        //{
        //case Topology::None:
        //    glDrawElements(GL_NONE, static_cast<GLsizei>(RectIndeces.size()), GL_UNSIGNED_INT, 0);
        //    break;
        //case Topology::Lines:
        //    glDrawElements(GL_LINES, static_cast<GLsizei>(RectIndeces.size()), GL_UNSIGNED_INT, 0);
        //    break;
        //case Topology::Triangles:
        //    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(RectIndeces.size()), GL_UNSIGNED_INT, 0);
        //    break;
        //default: ;
        //}
        
        // Swap the back buffer with the front buffer
        glfwSwapBuffers(Application::Get().GetWindow());
    }

    void OpenGLRenderer::InitShaders()
    {
        // Create Shader object
        shaderProgram = std::make_unique<OpenGLShader>("../shaders/default.vert", "../shaders/default.frag");
    }

    // Checks if the different Shaders have compiled properly
    void OpenGLShader::compileErrors(unsigned int shader, const char* type)
    {
        // Stores status of compilation
        GLint hasCompiled;
        // Character array to store error message in
        // char infoLog[1024];
        std::array<char, 1024> infoLog;
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
            if (hasCompiled == GL_FALSE)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog.data());
                std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog.data() << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
            if (hasCompiled == GL_FALSE)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog.data());
                std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog.data() << std::endl;
            }
        }
    }
}
