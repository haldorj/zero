#include "PointLight.h"

namespace Zero {

	PointLight::PointLight(glm::vec3 position, glm::vec3 color, float ambientIntensity, float diffuseIntensity,
		float con, float lin, float exp) : Light(color, ambientIntensity, diffuseIntensity)
	{
		m_Position = position;
		m_Constant = con;
		m_Linear = lin;
		m_Exponent = exp;
	}

}