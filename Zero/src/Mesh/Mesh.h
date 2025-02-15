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
		Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <OpenGLTexture>& textures);
		void Draw(OpenGLShader& shader);

	private:
		VAO m_VAO;

		std::vector <Vertex> m_Vertices;
		std::vector <GLuint> m_Indices;
		std::vector <OpenGLTexture> m_Textures;
	};
}