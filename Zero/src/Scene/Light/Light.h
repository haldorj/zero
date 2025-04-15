#pragma once

#include <glm/glm.hpp>

namespace Zero {
	class Light
	{
	public:
		Light() = default;
		~Light() = default;

		Light(glm::vec3 color, float ambientIntensity);

	private:
		glm::vec3 m_Color{ 1.0f };
		float m_AmbientIntensity{ 1.0f };
	};
}
