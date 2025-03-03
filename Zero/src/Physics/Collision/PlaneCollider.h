#pragma once
#include "Collider.h"

namespace Zero
{
    struct PlaneCollider : Collider
    {
        PlaneCollider(const glm::vec3& normal, const float distance)
            : Normal(normal), Distance(distance)
        {
            Type = ColliderType::Plane;
        }

        glm::vec3 FindFurthestPoint(const glm::vec3& direction) override
        {
            return {};
        }
		
        glm::vec3 Normal;
        float Distance;
    };
}
