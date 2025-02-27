#include "TestCollision.h"

#include "Scene/Transform.h"

namespace Zero
{
    CollisionPoints TestSphereToSphere(const Collider* sphereColliderA, const Transform* sphereTransformA,
                                       const Collider* sphereColliderB, const Transform* sphereTransformB)
    {
        const SphereCollider* sphereA = (SphereCollider*)sphereColliderA;
        const SphereCollider* sphereB = (SphereCollider*)sphereColliderB;

        const glm::vec3 centerA = sphereTransformA->Location;
        const glm::vec3 centerB = sphereTransformB->Location;

        const float distance = glm::distance(centerA, centerB);

        CollisionPoints points{};
        if (distance < 0.0001 || distance > sphereA->Radius + sphereB->Radius)
        {
            return points;
        }

        points.A = centerA - points.Normal * sphereA->Radius;
        points.B = centerB + points.Normal * sphereB->Radius;
        points.Normal = glm::normalize(centerA - centerB);
        points.Depth = sphereA->Radius + sphereB->Radius - distance;
        points.HasCollision = true;
        return points;
    }

    CollisionPoints TestSphereToPlane(const Collider* sphereCollider, const Transform* sphereTransform,
                                      const Collider* planeCollider, const Transform* planeTransform)
    {
        const SphereCollider* sphere = (SphereCollider*)sphereCollider;
        const PlaneCollider* plane = (PlaneCollider*)planeCollider;

        const glm::vec3 sphereCenter = sphereTransform->Location;
        const glm::vec3 planeNormal = glm::normalize(plane->Normal);

        const glm::vec3 pointOnPlane = planeTransform->Location;

        const float distance = glm::dot(sphereCenter - pointOnPlane, planeNormal);

        CollisionPoints points{};
        if (std::abs(distance) < sphere->Radius) // Correct condition check
        {
            points.A = sphereCenter - planeNormal * std::abs(distance);
            points.B = points.A - planeNormal * (sphere->Radius - std::abs(distance));
            points.Normal = planeNormal;
            points.Depth = sphere->Radius - std::abs(distance);
            points.HasCollision = true;
        }

        return points;
    }
}
