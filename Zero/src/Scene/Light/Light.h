#pragma once

#include <glm/glm.hpp>

namespace Zero {
	class Light
	{
	public:
		Light() = default;
		~Light() = default;

		Light(glm::vec3 color, float ambientIntensity, float diffuseIntensity);

		void Update(glm::vec3 color, float ambientIntensity, float diffuseIntensity);

		glm::vec3 GetColor() const { return m_Color; }
		float GetAmbientIntensity() const { return m_AmbientIntensity; }
		float GetDiffuseIntensity() const { return m_DiffuseIntensity; }

	protected:
		glm::vec3 m_Color{ 1.0f };
		float m_AmbientIntensity{ 1.0f };
		float m_DiffuseIntensity{ 1.0f };
	};
}
