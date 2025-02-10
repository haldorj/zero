#pragma once
#include <span>
#include "Vertex.h"

#include<string>

#include"Renderer/OpenGL/VAO.h"
#include"Renderer/OpenGL/EBO.h"
#include"Renderer/OpenGL/OpenGLTexture.h"
#include <glad/glad.h>
#include <Camera/Camera.h>

namespace Zero {
	class Mesh
	{
	public:
		std::vector <Vertex> vertices;
		std::vector <GLuint> indices;
		std::vector <OpenGLTexture> textures;
		// Store VAO in public so it can be used in the Draw function
		VAO VAO;

		// Initializes the mesh
		Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <OpenGLTexture>& textures);

		// Draws the mesh
		void Draw(OpenGLShader& shader, Camera& camera);
	};
}