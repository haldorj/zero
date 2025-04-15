#pragma once

#include <glm/glm.hpp>

namespace Zero {
	class Light
	{
	public:
		Light() = default;
		~Light() = default;

		Light(glm::vec3 color, float ambientIntensity);

		void Update(glm::vec3 color, float ambientIntensity);

		glm::vec3 GetColor() const { return m_Color; }
		float GetAmbientIntensity() const { return m_AmbientIntensity; }

	private:
		glm::vec3 m_Color{ 1.0f };
		float m_AmbientIntensity{ 1.0f };
	};
}
