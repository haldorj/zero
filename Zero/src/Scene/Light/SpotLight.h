#pragma once
#include "PointLight.h"

namespace Zero {

    class SpotLight : public PointLight
    {
    public:
		SpotLight() = default;

        SpotLight(glm::vec3 position, glm::vec3 color,
            			float ambientIntensity, float diffuseIntensity,
            			float con, float lin, float exp,
            			glm::vec3 direction, float edge);

        glm::vec3 GetDirection() const { return m_Direction; }
        float GetEdge() const { return m_ProcEdge; }

    private:
        glm::vec3 m_Direction{ 0.0f, 1.0f, 0.0f };
		float m_Edge{ 0.0f };
		float m_ProcEdge{ 0.0f };
    };

}
