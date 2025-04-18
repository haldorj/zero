#pragma once
#include "Light.h"

namespace Zero {

	class PointLight : public Light
	{
	public:
		PointLight() = default;

		PointLight(glm::vec3 position, glm::vec3 color, 
			float ambientIntensity, float diffuseIntensity,
			float con, float lin, float exp);

		glm::vec3 GetPosition() const { return m_Position; }
		float GetConstant() const { return m_Constant; }
		float GetLinear() const { return m_Linear; }
		float GetExponent() const { return m_Exponent; }

	private:
		glm::vec3 m_Position{};
		float m_Constant{};
		float m_Linear{};
		float m_Exponent{};
	};

}

