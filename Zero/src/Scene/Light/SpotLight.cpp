#include "SpotLight.h"

namespace Zero {

	SpotLight::SpotLight(glm::vec3 position, glm::vec3 color, float ambientIntensity, float diffuseIntensity, 
		float con, float lin, float exp, glm::vec3 direction, float edge) :
		PointLight(position, color, ambientIntensity, diffuseIntensity, con, lin, exp)
	{
		m_Direction = direction;
		m_Edge = edge;
		m_ProcEdge = cosf(glm::radians(m_Edge));
	}

}
