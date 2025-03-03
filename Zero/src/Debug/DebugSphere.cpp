#include "DebugSphere.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"

namespace Zero
{
    DebugSphere::DebugSphere(const unsigned int segments, const float radius)
        : m_Radius(radius), m_Segments(segments)
    {
        GenerateCircle(0, m_Segments);
        GenerateCircle(1, m_Segments);
        GenerateCircle(2, m_Segments);
    }

    void DebugSphere::GenerateCircle(const int axis, const unsigned int segments)
    {
        for (int i = 0; i < segments; ++i)
        {
            const float theta = M_PI * 2.0f * static_cast<float>(i) / static_cast<float>(segments);
            float x = m_Radius * cos(theta);
            float y = m_Radius * sin(theta);

            if (axis == 0) // XY Plane
                m_Vertices.emplace_back(x, y, 0.0f);
            else if (axis == 1) // XZ Plane
                m_Vertices.emplace_back(x, 0.0f, y);
            else if (axis == 2) // YZ Plane
                m_Vertices.emplace_back(0.0f, x, y);
        }
    }
}
