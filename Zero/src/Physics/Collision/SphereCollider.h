#pragma once
#include "Collider.h"
#include "Debug/DebugSphere.h"


namespace Zero
{
    struct SphereCollider : Collider
    {
    public:
        SphereCollider(const glm::vec3& center, const float radius)
            : Center(center), Radius(radius)
        {
            Sphere = DebugSphere(32, radius);
            Type = ColliderType::Sphere;
        }

        glm::vec3 FindFurthestPoint(const glm::vec3& direction) override
        {
            return {};
        }

        glm::vec3 Center;
        float Radius;
        DebugSphere Sphere{};
    };
}
