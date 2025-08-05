#include "DirectionalLight.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "Application.h"

namespace Zero {

    DirectionalLight::DirectionalLight(glm::vec3 color, float ambientIntensity,
        glm::vec3 direction, float diffuseIntensity)
        : Light(color, ambientIntensity, diffuseIntensity)
    {
        m_Direction = direction;

        float size = 30.0f;
        float nearPlane = 0.1f;
        float farPlane = 50.0f;

        m_ProjectionMatrix = glm::ortho(-size, size, -size, size, nearPlane, farPlane);

        if (Application::Get().GetRendererType() == RendererAPI::Vulkan)
        {
			// Vulkan clip space has inverted Y 
			// and half Z (converts from [-1,1] to [0,1] space).
            glm::mat4 clip = glm::mat4(1.0f);
			clip[1][1] = -1.0f;
			clip[2][2] = 0.5f;
			clip[3][2] = 0.5f;

            m_ProjectionMatrix = clip * m_ProjectionMatrix;
        }
    }

	void DirectionalLight::Update(glm::vec3 color, float ambientIntensity, 
		glm::vec3 direction, float diffuseIntensity)
	{
		m_Color = color;
		m_AmbientIntensity = ambientIntensity;
		m_DiffuseIntensity = diffuseIntensity;

		m_Direction = direction;
	}

	glm::mat4 DirectionalLight::GetLightTransform() const
	{
		glm::vec3 origin = glm::vec3{0.0f};
		glm::vec3 up = glm::vec3{ 0.0f, 1.0f, 0.0f };

		glm::vec3 position = glm::normalize(m_Direction) * 30.0f;

		return m_ProjectionMatrix * glm::lookAt(position, origin, up);
	}

}