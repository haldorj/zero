#pragma once
#include "Light.h"

namespace Zero {

	class DirectionalLight : public Light
	{
	public:
		DirectionalLight() = default;
		DirectionalLight(glm::vec3 color, float ambientIntensity,
			glm::vec3 direction, float diffuseIntensity);

		void Update(glm::vec3 color, float ambientIntensity,
			glm::vec3 direction, float diffuseIntensity);

		glm::mat4 GetLightTransform() const;

		glm::vec3 GetDirection() const { return m_Direction; }

	private:
		glm::vec3 m_Direction{ 0.0f, 1.0f, 0.0f };
		glm::mat4 m_ProjectionMatrix{};
	};

}

