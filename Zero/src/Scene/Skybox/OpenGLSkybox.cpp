#include "OpenGLSkybox.h"
#include <glm/gtc/type_ptr.hpp>


namespace Zero {

	void OpenGLSkybox::Draw(const glm::mat4& projection, const glm::mat4& view)
	{
		glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_LEQUAL);

		m_ShaderProgram->Activate();
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram->GetID(), "view"), 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram->GetID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
	}

	void OpenGLSkybox::LoadCubeMap(const std::vector<std::string>& faceLocations)
	{
		stbi_set_flip_vertically_on_load(false);

		m_ShaderProgram = new OpenGLShader(
			"../shaders/opengl/skybox.vert",
			"../shaders/opengl/skybox.frag"
		);

		glGenTextures(1, &m_TextureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

		int width, height, numChannels;
		unsigned char* data;

		for (unsigned int i = 0; i < faceLocations.size(); i++)
		{
			data = stbi_load(faceLocations[i].c_str(), &width, &height, &numChannels, 0);

			if (!data)
			{
				std::cerr << "Cubemap texture failed to load at path: " << faceLocations[i] << std::endl;
				continue;
			}

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


			stbi_image_free(data);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		unsigned int skyboxIndices[] = {
			// front
			0, 1, 2,
			2, 1, 3,
			// right
			2, 3, 5,
			5, 3, 7,
			// back
			5, 7, 4,
			4, 7, 6,
			// left
			4, 6, 0,
			0, 6, 1,
			// top
			4, 0, 5,
			5, 0, 2,
			// bottom
			1, 6, 3,
			3, 6, 7
		};

		float skyboxVertices[] = {
			-1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
		};

		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glGenBuffers(1, &skyboxEBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}

