#include "OpenGLRenderer.h"
#include <core/core.h>
#include <core/Application.h>
#include <iostream>
#include <stb_image.h>

std::vector<GLfloat> RectVertices;
std::vector<GLuint> RectIndeces;

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
	}



	void OpenGLRenderer::InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices)
	{
		for (auto &vertex : vertices)
		{
			RectVertices.push_back((GLfloat)vertex.Position.x); 
			RectVertices.push_back((GLfloat)vertex.Position.y);
			RectVertices.push_back((GLfloat)vertex.Position.z);
			RectVertices.push_back((GLfloat)vertex.Color.r);
			RectVertices.push_back((GLfloat)vertex.Color.g);
			RectVertices.push_back((GLfloat)vertex.Color.b);
			RectVertices.push_back((GLfloat)vertex.UvX);
			RectVertices.push_back((GLfloat)vertex.UvY);
		}
		for (auto index : indices)
		{
			RectIndeces.push_back((GLuint)index);
		}
	}

	void OpenGLRenderer::Shutdown()
	{
		// Delete all the objects we've created
		VAO1->Delete();
		VBO1->Delete();
		EBO1->Delete();
		shaderProgram->Delete();
		glDeleteTextures(1, &textureID);
	}

	void OpenGLRenderer::Draw()
	{
		// Specify the color of the background
		glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
		// Clean the back buffer and assign the new color to it
		glClear(GL_COLOR_BUFFER_BIT);
		// Tell OpenGL which Shader Program we want to use
		shaderProgram->Activate();
		// Scale uniform. Must be called after the Shader Program has been activated.
		glUniform1f(uniID, 1.f);
		glBindTexture(GL_TEXTURE_2D, textureID);
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

		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load("../assets/images/cat.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			std::cout << "Failed to load texture file" << std::endl;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(pixels);
		glBindTexture(GL_TEXTURE_2D, 0);

		GLuint texUniID = glGetUniformLocation(shaderProgram->GetID(), "tex0");
		shaderProgram->Activate();
		glUniform1i(texUniID, 0);
	}

	// Checks if the different Shaders have compiled properly
	void OpenGLShader::compileErrors(unsigned int shader, const char* type)
	{
		// Stores status of compilation
		GLint hasCompiled;
		// Character array to store error message in
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
			if (hasCompiled == GL_FALSE)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
			if (hasCompiled == GL_FALSE)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
			}
		}
	}

}

