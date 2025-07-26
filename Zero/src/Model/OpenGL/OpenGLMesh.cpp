#include "OpenGLMesh.h"
#include <glm/gtc/type_ptr.hpp>

namespace Zero
{
    OpenGLMesh::OpenGLMesh(const std::vector<Vertex>& vertices, std::vector<GLuint>& indices,
                           const std::vector<OpenGLTexture>& textures)
    {
        m_Vertices = vertices;
        m_Indices = indices;
        m_Textures = textures;

        if (m_Textures.empty())
        {
            // If no textures are loaded, load a default texture
            const OpenGLTexture defaultTexture("assets/images/plain.png", "texture_diffuse", 0, GL_UNSIGNED_BYTE);
            m_Textures.emplace_back(defaultTexture);
        }

        m_VAO.Bind();
        // Generates Vertex Buffer Object and links it to vertices
        VBO vbo(m_Vertices);
        // Generates Element Buffer Object and links it to indices
        EBO ebo(indices);
        // Links VBO attributes such as coordinates and colors to VAO

        m_VAO.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        m_VAO.LinkAttrib(vbo, 1, 1, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, UvX));
        m_VAO.LinkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        m_VAO.LinkAttrib(vbo, 3, 1, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, UvY));
        m_VAO.LinkAttrib(vbo, 4, 4, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Color));
        m_VAO.LinkAttribINT(vbo, 5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
        m_VAO.LinkAttrib(vbo, 6, 4, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

        // Unbind all to prevent accidentally modifying them
        m_VAO.Unbind();
        vbo.Unbind();
        ebo.Unbind();
    }


    void OpenGLMesh::Draw(OpenGLShader& shader, glm::mat4& matrix)
    {
        // Bind shader to be able to access uniforms
        const int modelLoc = glGetUniformLocation(shader.GetID(), "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(matrix));

        shader.Activate();
        m_VAO.Bind();

        // Keep track of how many of each type of textures we have
        unsigned int numDiffuse = 0;
        unsigned int numSpecular = 0;

        for (unsigned int i = 0; i < m_Textures.size(); i++)
        {
            std::string num;
            std::string type = m_Textures[i].GetType();
            if (type == "texture_diffuse")
            {
                num = std::to_string(numDiffuse++);
            }
            else if (type == "texture_specular")
            {
                num = std::to_string(numSpecular++);
            }
            m_Textures[i].TexUnit(shader, type + num, i);
            m_Textures[i].Bind();
        }

        // Draw the actual mesh
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, nullptr);
    }
}
