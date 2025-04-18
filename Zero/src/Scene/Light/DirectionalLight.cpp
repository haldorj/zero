#include "DirectionalLight.h"

namespace Zero {

	DirectionalLight::DirectionalLight(glm::vec3 color, float ambientIntensity,
		glm::vec3 direction, float diffuseIntensity) : Light(color, ambientIntensity, diffuseIntensity)
	{
		m_Direction = direction;
	}

	void DirectionalLight::Update(glm::vec3 color, float ambientIntensity, 
		glm::vec3 direction, float diffuseIntensity)
	{
		m_Color = color;
		m_AmbientIntensity = ambientIntensity;
		m_DiffuseIntensity = diffuseIntensity;

		m_Direction = direction;
	}
}