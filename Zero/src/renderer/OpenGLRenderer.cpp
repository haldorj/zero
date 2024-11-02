#include "OpenGLRenderer.h"
#include <core/core.h>
#include <core/Application.h>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Vertices coordinates
GLfloat vertices[] =
{   //     COORDINATES     /        COLORS          /    TexCoord   /        NORMALS       //
    -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side
    -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
     0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 5.0f,      0.0f, -1.0f, 0.0f, // Bottom side
     0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, -1.0f, 0.0f, // Bottom side

    -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
    -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,     -0.8f, 0.5f,  0.0f, // Left Side
     0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,     -0.8f, 0.5f,  0.0f, // Left Side

    -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
     0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 0.0f,      0.0f, 0.5f, -0.8f, // Non-facing side
     0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.0f, 0.5f, -0.8f, // Non-facing side

     0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	 0.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
     0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.8f, 0.5f,  0.0f, // Right side
     0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.8f, 0.5f,  0.0f, // Right side

     0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	 5.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
    -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f, 	 0.0f, 0.0f,      0.0f, 0.5f,  0.8f, // Facing side
     0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	 2.5f, 5.0f,      0.0f, 0.5f,  0.8f  // Facing side
};

// Indices for vertices order
GLuint indices[] =
{
    0, 1, 2, // Bottom side
    0, 2, 3, // Bottom side
    4, 6, 5, // Left side
    7, 9, 8, // Non-facing side
    10, 12, 11, // Right side
    13, 15, 14 // Facing side
};

GLfloat lightVertices[] =
{ //     COORDINATES     //
    -0.1f, -0.1f,  0.1f,
    -0.1f, -0.1f, -0.1f,
     0.1f, -0.1f, -0.1f,
     0.1f, -0.1f,  0.1f,
    -0.1f,  0.1f,  0.1f,
    -0.1f,  0.1f, -0.1f,
     0.1f,  0.1f, -0.1f,
     0.1f,  0.1f,  0.1f
};

GLuint lightIndices[] =
{
    0, 1, 2,
    0, 2, 3,
    0, 4, 7,
    0, 7, 3,
    3, 7, 6,
    3, 6, 2,
    2, 6, 5,
    2, 5, 1,
    1, 5, 4,
    1, 4, 0,
    4, 5, 6,
    4, 6, 7
};

namespace Zero
{
    void OpenGLRenderer::Init()
    {
        std::cout << "ZeroEngine OpenGL \n";

        glfwMakeContextCurrent(Application::Get().GetWindow());

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

        glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, lightPos);

        glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::mat4 pyramidModel = glm::mat4(1.0f);
        pyramidModel = glm::translate(pyramidModel, pyramidPos);


        lightShaderProgram->Activate();
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram->GetID(), "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
        glUniform4f(glGetUniformLocation(lightShaderProgram->GetID(), "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
        shaderProgram->Activate();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram->GetID(), "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
        glUniform4f(glGetUniformLocation(shaderProgram->GetID(), "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
        glUniform3f(glGetUniformLocation(shaderProgram->GetID(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);

        texture1 = std::make_shared<OpenGLTexture>("../assets/images/brick.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA,
                                                   GL_UNSIGNED_BYTE);
        texture1->TexUnit(*shaderProgram, "tex0", 0);

        // Enable depth testing for 3D
        glEnable(GL_DEPTH_TEST);
    }


    void OpenGLRenderer::InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices)
    {
        //for (const auto& vertex : vertices)
        //{
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetPosition().x));
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetPosition().y));
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetPosition().z));
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetNormal().x));
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetNormal().y));
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetNormal().z));
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetTexCoord().x));
        //    RectVertices.push_back(static_cast<GLfloat>(vertex.GetTexCoord().y));
        //}
        //for (const auto index : indices)
        //{
        //    RectIndeces.push_back(static_cast<GLuint>(index));
        //}
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

        glUniform3f(glGetUniformLocation(shaderProgram->GetID(), "camPos"), Application::Get().GetMainCamera().GetPosition().x,
					Application::Get().GetMainCamera().GetPosition().y, Application::Get().GetMainCamera().GetPosition().z);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

       
        view = Application::Get().GetMainCamera().GetViewMatrix();
        projection = glm::perspective(glm::radians(70.0f), (float)EXTENT_WIDTH / (float)EXTENT_HEIGHT, 0.1f, 100.0f);

        int viewLoc = glGetUniformLocation(shaderProgram->GetID(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projLoc = glGetUniformLocation(shaderProgram->GetID(), "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Bind the texture so that it is used in the Shader Program
        texture1->Bind();
        // Bind the VAO so OpenGL knows to use it
        VAO1->Bind();
        // Draw primitives, number of indices, datatype of indices, index of indices
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_INT, 0);

        // Tells OpenGL which Shader Program we want to use
        lightShaderProgram->Activate();

        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram->GetID(), "camMatrix"), 1, GL_FALSE, glm::value_ptr(projection * Application::Get().GetMainCamera().GetViewMatrix()));
        // Bind the VAO so OpenGL knows to use it
        lightVAO->Bind();
        // Draw primitives, number of indices, datatype of indices, index of indices
        glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

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
        VBO1 = std::make_unique<VBO>(vertices, sizeof(vertices));
        // Generates Element Buffer Object and links it to indices
        EBO1 = std::make_unique<EBO>(indices, sizeof(indices));

        // Links VBO attributes such as coordinates and colors to VAO
        VAO1->LinkAttrib(*VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0); // Vertex positions
        VAO1->LinkAttrib(*VBO1, 1, 3, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float))); // Vertex colors
        VAO1->LinkAttrib(*VBO1, 2, 2, GL_FLOAT, 11 * sizeof(float), (void*)(6 * sizeof(float))); // Texture coordinates
        VAO1->LinkAttrib(*VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float))); // Vertex normals
        // Unbind all to prevent accidentally modifying them
        VAO1->Unbind();
        VBO1->Unbind();
        EBO1->Unbind();

        uniID = glGetUniformLocation(shaderProgram->GetID(), "scale");

        lightShaderProgram = std::make_unique<OpenGLShader>("../shaders/light.vert", "../shaders/light.frag");

        lightVAO = new VAO();
        lightVAO->Bind();
        // Generates Vertex Buffer Object and links it to vertices
        lightVBO = new VBO(lightVertices, sizeof(lightVertices));
        // Generates Element Buffer Object and links it to indices
        lightEBO = new EBO(lightIndices, sizeof(lightIndices));
        // Links VBO attributes such as coordinates and colors to VAO
        lightVAO->LinkAttrib(*lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
        // Unbind all to prevent accidentally modifying them
        lightVAO->Unbind();
        lightVBO->Unbind();
        lightEBO->Unbind();
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
