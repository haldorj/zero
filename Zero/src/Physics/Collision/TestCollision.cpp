#include "TestCollision.h"

#include "Scene/Transform.h"

namespace Zero
{
    CollisionPoints TestSphereToSphere(const Collider* sphereColliderA, const Transform* sphereTransformA,
                                       const Collider* sphereColliderB, const Transform* sphereTransformB)
    {
        const auto sphereA = dynamic_cast<const SphereCollider*>(sphereColliderA);
        const auto sphereB = dynamic_cast<const SphereCollider*>(sphereColliderB);

        const glm::vec3 centerA = sphereTransformA->Position + sphereA->Center;
        const glm::vec3 centerB = sphereTransformB->Position + sphereB->Center;

        const float distance = glm::distance(centerA, centerB);

        CollisionPoints points{};
        if (distance < 0.0001 || distance > sphereA->Radius + sphereB->Radius)
        {
            return points;
        }
        
        glm::vec3 normal{};

        if (centerA == centerB)
        {
            normal = {0.f,1.f,0.f};
        }
        else
        {
            normal = glm::normalize(centerA - centerB);
        }

        points.ADeep = centerA - normal * sphereA->Radius;
        points.BDeep = centerB + normal * sphereB->Radius;
        points.Normal = normal;
        points.Depth = sphereA->Radius + sphereB->Radius - distance;
        points.HasCollision = true;
        return points;
    }

    CollisionPoints TestSphereToPlane(const Collider* sphereCollider, const Transform* sphereTransform,
                                      const Collider* planeCollider, const Transform* planeTransform)
    {
        const auto sphere = dynamic_cast<const SphereCollider*>(sphereCollider);
        const auto plane = dynamic_cast<const PlaneCollider*>(planeCollider);

        const glm::vec3 sphereCenter = sphereTransform->Position + sphere->Center;
        const glm::vec3 planeNormal = glm::normalize(plane->Normal);

        const glm::vec3 pointOnPlane = planeTransform->Position;

        const float distance = glm::dot(sphereCenter - pointOnPlane, planeNormal);

        CollisionPoints points{};
        if (distance < sphere->Radius)
        {
            points.ADeep = sphereCenter - planeNormal * sphere->Radius;
            points.BDeep = sphereCenter - planeNormal * distance;
            points.Normal = planeNormal;
            points.Depth = sphere->Radius - std::abs(distance);
            points.HasCollision = true;
        }
        return points;
    }
}
