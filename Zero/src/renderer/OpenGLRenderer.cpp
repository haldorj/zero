#include "OpenGLRenderer.h"
#include <core/core.h>
#include <core/Application.h>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<GLfloat> RectVertices;
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

        texture1 = std::make_shared<OpenGLTexture>("../assets/images/brick.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA,
                                                   GL_UNSIGNED_BYTE);
        texture1->TexUnit(*shaderProgram, "tex0", 0);

        // Enable depth testing for 3D
        glEnable(GL_DEPTH_TEST);
    }


    void OpenGLRenderer::InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices)
    {
        for (const auto& vertex : vertices)
        {
            RectVertices.push_back(static_cast<GLfloat>(vertex.Position.x));
            RectVertices.push_back(static_cast<GLfloat>(vertex.Position.y));
            RectVertices.push_back(static_cast<GLfloat>(vertex.Position.z));
            RectVertices.push_back(static_cast<GLfloat>(vertex.Color.r));
            RectVertices.push_back(static_cast<GLfloat>(vertex.Color.g));
            RectVertices.push_back(static_cast<GLfloat>(vertex.Color.b));
            RectVertices.push_back(static_cast<GLfloat>(vertex.UvX));
            RectVertices.push_back(static_cast<GLfloat>(vertex.UvY));
        }
        for (const auto index : indices)
        {
            RectIndeces.push_back(static_cast<GLuint>(index));
        }
    }

    void OpenGLRenderer::Shutdown()
    {
        // Delete all the objects we've created
        VAO1->Delete();
        VBO1->Delete();
        EBO1->Delete();
        shaderProgram->Delete();
        texture1->Delete();
    }

    float rotation = 0.0f;
    double lastTime = glfwGetTime();

    void OpenGLRenderer::Draw()
    {
        // Specify the color of the background
        glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
        // Clean the back buffer and assign the new color to it
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Tell OpenGL which Shader Program we want to use
        shaderProgram->Activate();

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

        // Scale uniform. Must be called after the Shader Program has been activated.
        glUniform1f(uniID, 1.f);
        // Bind the texture so that it is used in the Shader Program
        texture1->Bind();
        // Bind the VAO so OpenGL knows to use it
        VAO1->Bind();
        // Draw primitives, number of indices, datatype of indices, index of indices
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(RectIndeces.size()), GL_UNSIGNED_INT, 0);
        // Swap the back buffer with the front buffer
        glfwSwapBuffers(Application::Get().GetWindow());
    }

    void OpenGLRenderer::InitShaders()
    {
        // Create Shader object
        shaderProgram = std::make_unique<OpenGLShader>("../shaders/default.vert", "../shaders/default.frag");

        // Generates Vertex Array Object and binds it
        VAO1 = std::make_unique<VAO>();
        VAO1->Bind();


        // Generates Vertex Buffer Object and links it to vertices
        VBO1 = std::make_unique<VBO>(RectVertices.data(), RectVertices.size() * sizeof(float));
        // Generates Element Buffer Object and links it to indices
        EBO1 = std::make_unique<EBO>(RectIndeces.data(), RectIndeces.size() * sizeof(GLuint));

        // Links VBO attributes such as coordinates and colors to VAO
        VAO1->LinkAttrib(*VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0); // Vertex positions
        VAO1->LinkAttrib(*VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float))); // Vertex colors
        VAO1->LinkAttrib(*VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float))); // Texture coordinates
        // Unbind all to prevent accidentally modifying them
        VAO1->Unbind();
        VBO1->Unbind();
        EBO1->Unbind();

        uniID = glGetUniformLocation(shaderProgram->GetID(), "scale");
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
