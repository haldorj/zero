#include "OpenGLShadowmap.h"
#include "Core/core.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Zero
{
	OpenGLShadowmap::OpenGLShadowmap()
	{
		Init();
	}

	OpenGLShadowmap::~OpenGLShadowmap()
	{
		if (m_DepthMapFBO)
			glDeleteFramebuffers(1, &m_DepthMapFBO);

		if (m_DepthMapTexture)
			glDeleteTextures(1, &m_DepthMapTexture);
	}

	void OpenGLShadowmap::Init()
	{
		m_ShaderProgram = new OpenGLShader(
			"../shaders/OpenGL/shadowmap.vert",
			"../shaders/OpenGL/shadowmap.frag"
		);

		// Create a framebuffer object (FBO) that will hold the depth map:
		glGenFramebuffers(1, &m_DepthMapFBO);

		// Create a 2D texture that we'll use as the framebuffer's depth buffer:
		glGenTextures(1, &m_DepthMapTexture);
		glBindTexture(GL_TEXTURE_2D, m_DepthMapTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

		// Attach the texture to the framebuffer:
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthMapTexture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("Shadowmap Framebuffer Error: %i \n", status);
			return;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

    void OpenGLShadowmap::Draw(Scene* scene)  
    {  
       glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);  
       glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);  
       glClear(GL_DEPTH_BUFFER_BIT);

       m_ShaderProgram->Activate();

	   auto& directionalLight = *(scene->GetDirectionalLight());

       int viewLoc = glGetUniformLocation(m_ShaderProgram->GetID(), "lightTransform");  
       glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(directionalLight.GetLightTransform()));  

       for (auto& gameObj : scene->GetGameObjects())  
       {  
           if (!gameObj->GetModel())  
               continue;

		   if (gameObj->GetAnimator())
		   {
			   glUniform1i(glGetUniformLocation(m_ShaderProgram->GetID(), "Animated"), 1);

			   const std::vector transforms = gameObj->GetAnimator()->GetFinalBoneMatrices();
			   for (int i = 0; i < transforms.size(); ++i)
			   {
				   std::string str = "finalBonesMatrices[" + std::to_string(i) + "]";

				   int transformLoc = glGetUniformLocation(m_ShaderProgram->GetID(), str.c_str());
				   glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transforms[i]));
			   }
		   }
		   else
		   {
			   glUniform1i(glGetUniformLocation(m_ShaderProgram->GetID(), "Animated"), 0);
		   }

           glm::mat4 model = gameObj->GetTransform().GetMatrix();  
           glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram->GetID(), "model"), 1, GL_FALSE, glm::value_ptr(model));  
           gameObj->GetModel()->Draw(*m_ShaderProgram, model);  
       }  

       glBindFramebuffer(GL_FRAMEBUFFER, 0);  
    }
}