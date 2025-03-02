#pragma once
#include <span>

#include<string>

#include "Model/Vertex.h"
#include"Renderer/OpenGL/VAO.h"
#include"Renderer/OpenGL/EBO.h"
#include"Renderer/OpenGL/OpenGLTexture.h"
#include <glad/glad.h>
#include <Camera/EditorCamera.h>

namespace Zero
{
    class OpenGLMesh
    {
    public:
        OpenGLMesh(const std::vector<Vertex>& vertices, std::vector<GLuint>& indices,
                   const std::vector<OpenGLTexture>& textures);
        void Draw(OpenGLShader& shader, glm::mat4& matrix);

    private:
        VAO m_VAO;

        std::vector<Vertex> m_Vertices;
        std::vector<GLuint> m_Indices;
        std::vector<OpenGLTexture> m_Textures;
    };
}
