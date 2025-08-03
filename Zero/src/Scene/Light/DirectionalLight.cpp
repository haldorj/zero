#include "DirectionalLight.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "Application.h"

namespace Zero {

	DirectionalLight::DirectionalLight(glm::vec3 color, float ambientIntensity,
		glm::vec3 direction, float diffuseIntensity) : Light(color, ambientIntensity, diffuseIntensity)
	{
		m_Direction = direction;

		float nearPlane = 1.0f, farPlane = 20.0f;
		float size = 30.0f; // Size of the orthographic projection
		m_ProjectionMatrix = glm::ortho(-size, size, -size, size, nearPlane, farPlane);

		if (Application::Get().GetRendererType() == RendererAPI::Vulkan)
		{
			m_ProjectionMatrix[1][1] *= -1; // Invert Y-axis for Vulkan
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

		glm::vec3 position = glm::normalize(m_Direction) * 10.0f;

		return m_ProjectionMatrix * glm::lookAt(position, origin, up);
	}

}