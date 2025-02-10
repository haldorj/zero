#include "Mesh.h"

#include "Mesh.h"

namespace Zero {
	Mesh::Mesh(std::vector <Vertex>& vertices, std::vector<GLuint>& indices, std::vector <OpenGLTexture>& textures)
	{
		Mesh::vertices = vertices;
		Mesh::indices = indices;
		Mesh::textures = textures;

		VAO.Bind();
		// Generates Vertex Buffer Object and links it to vertices
		VBO VBO(vertices);
		// Generates Element Buffer Object and links it to indices
		EBO EBO(indices);
		// Links VBO attributes such as coordinates and colors to VAO
		VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);                          // Pos (vec3) -> Offset 0
		VAO.LinkAttrib(VBO, 1, 1, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));        // UV_x (float) -> Offset 3 * sizeof(float)
		VAO.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(4 * sizeof(float)));        // Normal (vec3) -> Offset 4 * sizeof(float)
		VAO.LinkAttrib(VBO, 3, 1, GL_FLOAT, sizeof(Vertex), (void*)(7 * sizeof(float)));        // UV_y (float) -> Offset 7 * sizeof(float)
		VAO.LinkAttrib(VBO, 4, 4, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float)));        // Color (vec4) -> Offset 8 * sizeof(float)

		// Unbind all to prevent accidentally modifying them
		VAO.Unbind();
		VBO.Unbind();
		EBO.Unbind();
	}


	void Mesh::Draw(OpenGLShader& shader, Camera& camera)
	{
		// Bind shader to be able to access uniforms
		shader.Activate();
		VAO.Bind();

		// Keep track of how many of each type of textures we have
		unsigned int numDiffuse = 0;
		unsigned int numSpecular = 0;

		for (unsigned int i = 0; i < textures.size(); i++)
		{
			std::string num;
			std::string type = textures[i].GetType();
			if (type == "diffuse")
			{
				num = std::to_string(numDiffuse++);
			}
			else if (type == "specular")
			{
				num = std::to_string(numSpecular++);
			}
			textures[i].TexUnit(shader, (type + num).c_str(), i);
			textures[i].Bind();
		}

		// Draw the actual mesh
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}
}