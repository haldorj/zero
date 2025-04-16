#pragma once

#include <glm/glm.hpp>

namespace Zero {
	class Light
	{
	public:
		Light() = default;
		~Light() = default;

		Light(glm::vec3 color, float ambientIntensity,
		glm::vec3 direction, float diffuseIntensity);

		void Update(glm::vec3 color, float ambientIntensity,
			glm::vec3 direction, float diffuseIntensity);

		glm::vec3 GetColor() const { return m_Color; }
		float GetAmbientIntensity() const { return m_AmbientIntensity; }

		glm::vec3 GetDirection() const { return m_Direction; }
		float GetDiffuseIntensity() const { return m_DiffuseIntensity; }

	private:
		glm::vec3 m_Color{ 1.0f };
		float m_AmbientIntensity{ 1.0f };

		glm::vec3 m_Direction{ 0.0f, -1.0f, 0.0f };
		float m_DiffuseIntensity{ 1.0f };
	};
}
