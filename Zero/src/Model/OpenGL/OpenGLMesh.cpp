#include "OpenGLMesh.h"
#include <glm/gtc/type_ptr.hpp>

namespace Zero {
	OpenGLMesh::OpenGLMesh(std::vector <Vertex>& vertices, std::vector<GLuint>& indices, std::vector <OpenGLTexture>& textures)
	{
		m_Vertices = vertices;
		m_Indices = indices;
		m_Textures = textures;

		if (m_Textures.size() <= 0)
		{
			// If no textures are loaded, load a default texture
			OpenGLTexture defaultTexture("../assets/images/plain.png", "texture_diffuse", 0, GL_UNSIGNED_BYTE);
			m_Textures.push_back(defaultTexture);
		}

		m_VAO.Bind();
		// Generates Vertex Buffer Object and links it to vertices
		VBO VBO(m_Vertices);
		// Generates Element Buffer Object and links it to indices
		EBO EBO(indices);
		// Links VBO attributes such as coordinates and colors to VAO
		m_VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);                          // Pos (vec3) -> Offset 0
		m_VAO.LinkAttrib(VBO, 1, 1, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));        // UV_x (float) -> Offset 3 * sizeof(float)
		m_VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(4 * sizeof(float)));        // Normal (vec3) -> Offset 4 * sizeof(float)
		m_VAO.LinkAttrib(VBO, 3, 1, GL_FLOAT, sizeof(Vertex), (void*)(7 * sizeof(float)));        // UV_y (float) -> Offset 7 * sizeof(float)
		m_VAO.LinkAttrib(VBO, 4, 4, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float)));        // Color (vec4) -> Offset 8 * sizeof(float)

		// Unbind all to prevent accidentally modifying them
		m_VAO.Unbind();
		VBO.Unbind();
		EBO.Unbind();
	}


	void OpenGLMesh::Draw(OpenGLShader& shader, glm::mat4& matrix)
	{
		// Bind shader to be able to access uniforms
		int modelLoc = glGetUniformLocation(shader.GetID(), "model");
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
			m_Textures[i].TexUnit(shader, (type + num).c_str(), i);
			m_Textures[i].Bind();
		}

		// Draw the actual mesh
		glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
	}
}