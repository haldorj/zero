#pragma once
#include "Collider.h"

namespace Zero
{
    struct MeshCollider : Collider
    {
        MeshCollider() = default;

        glm::vec3 FindFurthestPoint(const glm::vec3& direction) override
        {
            glm::vec3 maxPoint{};
            float maxDistance = -FLT_MAX;

            for (const auto& vertex : Vertices)
            {
                const float distance = glm::dot(vertex, direction);
                if (distance > maxDistance)
                {
                    maxDistance = distance;
                    maxPoint = vertex;
                }
            }
			
            return maxPoint;
        }
		
        std::vector <glm::vec3> Vertices;
    };
}
