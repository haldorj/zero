#pragma once
#include "glm/glm.hpp"
#include "Scene/Transform.h"

namespace Zero
{
    class DebugSphere
    {
    public:
        DebugSphere() = default;
        ~DebugSphere() = default;
        DebugSphere(const unsigned int segments, const float radius);
        
        void GenerateCircle(const int axis, const unsigned int segments);
        void SetRadius(const float radius) { m_Radius = radius; }

        void Draw(Transform) const;

    private:
        glm::vec3 m_Position{0.0f};
        float m_Radius = 1.0f;
        unsigned int m_Segments;

        std::vector<glm::vec3> m_Vertices;
    };
}

