#pragma once
#include "Collider.h"

namespace Zero
{
    struct CapsuleCollider : Collider
    {
        CapsuleCollider(const glm::vec3& center, const float radius, const float height)
            : Center(center), Radius(radius), Height(height)
        {
            Type = ColliderType::Capsule;
        }

        glm::vec3 FindFurthestPoint(const glm::vec3& direction) override
        {
            return {};
        }
		
        glm::vec3 Center;
        float Radius;
        float Height;
    };
}
