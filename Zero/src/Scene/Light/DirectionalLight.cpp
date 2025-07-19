#include "DirectionalLight.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Zero {

	DirectionalLight::DirectionalLight(glm::vec3 color, float ambientIntensity,
		glm::vec3 direction, float diffuseIntensity) : Light(color, ambientIntensity, diffuseIntensity)
	{
		m_Direction = direction;

		float nearPlane = 1.0f, farPlane = 120.0f;
		m_ProjectionMatrix = glm::ortho(-45.0f, 45.0f, -45.0f, 45.0f, nearPlane, farPlane);
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

		glm::vec3 position = glm::normalize(m_Direction) * 40.0f;

		return m_ProjectionMatrix * glm::lookAt(position, origin, up);
	}

}