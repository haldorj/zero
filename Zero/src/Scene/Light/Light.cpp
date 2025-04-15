#include "Light.h"

Zero::Light::Light(glm::vec3 color, float ambientIntensity,
	glm::vec3 direction, float diffuseIntensity)
	: m_Color(color), m_AmbientIntensity(ambientIntensity),
	m_Direction(direction), m_DiffuseIntensity(diffuseIntensity)
{
}

void Zero::Light::Update(glm::vec3 color, float ambientIntensity)
{
	m_Color = color;
	m_AmbientIntensity = ambientIntensity;
}
