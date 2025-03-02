#pragma once

#include "Simplex.h"
#include "Physics/Collision/Collider.h"

namespace Zero
{
    inline glm::vec3 Support(
        Collider* colliderA,
        Collider* colliderB,
        const glm::vec3& direction)
    {
        return colliderA->FindFurthestPoint(direction) - colliderB->FindFurthestPoint(-direction);
    }
    
    inline bool GJK(Collider* colliderA, Collider* colliderB)
    {
        // Get initial support point in any direction
        glm::vec3 support = Support(colliderA, colliderB, glm::vec3(1.0f, 0.0f, 0.0f));

        // Simplex is an array of max 4 points (tetrahedron)
        Simplex simplex{};
        simplex.PushFront(support);

        // New direction is towards the origin
        const glm::vec3 direction = -support;
        while (true)
        {
            support = Support(colliderA, colliderB, direction);
            if (glm::dot(support, direction) <= 0)
            {
                // No collision
                return false;
            }
            simplex.PushFront(support);
            if (NextSimplex(simplex, direction))
            {
                return true;
            }
        }
    }
}
