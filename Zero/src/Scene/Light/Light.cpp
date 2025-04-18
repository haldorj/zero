#include "Light.h"

Zero::Light::Light(glm::vec3 color, float ambientIntensity, float diffuseIntensity)
	: m_Color(color), m_AmbientIntensity(ambientIntensity),
	m_DiffuseIntensity(diffuseIntensity)
{
}

void Zero::Light::Update(glm::vec3 color, float ambientIntensity,
	float diffuseIntensity)
{
	m_Color = color;
	m_AmbientIntensity = ambientIntensity;
	m_DiffuseIntensity = diffuseIntensity;
}
