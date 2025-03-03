#include "TestCollision.h"

#include "CapsuleCollider.h"
#include "PlaneCollider.h"
#include "SphereCollider.h"
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
            normal = {0.f, 1.f, 0.f};
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
        if (glm::abs(distance) < sphere->Radius)
        {
            points.ADeep = sphereCenter - planeNormal * sphere->Radius;
            points.BDeep = sphereCenter - planeNormal * glm::abs(distance);
            points.Normal = planeNormal;
            points.Depth = sphere->Radius - glm::abs(distance);
            points.HasCollision = true;
        }
        return points;
    }

    CollisionPoints TestCapsuleToCapsule(const Collider* capsuleColliderA, const Transform* capsuleTransformA,
                                         const Collider* capsuleColliderB, const Transform* capsuleTransformB)
    {
        return {};
    }

    CollisionPoints TestPlaneToCapsule(const Collider* planeCollider, const Transform* planeTransform,
                                       const Collider* capsuleCollider, const Transform* capsuleTransform)
    {
        const auto capsule = dynamic_cast<const CapsuleCollider*>(capsuleCollider);
        const auto plane = dynamic_cast<const PlaneCollider*>(planeCollider);

        const glm::vec3 capsuleCenter = capsuleTransform->Position + capsule->Center;
        const float capsuleRadius = capsule->Radius;
        const float capsuleHeight = capsule->Height;

        // Calculate the normal of the plane
        const glm::vec3 planeNormal = glm::normalize(plane->Normal);
    
        // Calculate the two end points of the capsule
        const glm::vec3 top = capsuleCenter + glm::vec3(0.f, capsuleHeight / 2.f, 0.f);
        const glm::vec3 bottom = capsuleCenter - glm::vec3(0.f, capsuleHeight / 2.f, 0.f);

        // Calculate the distances from the plane to the top and bottom points of the capsule
        const float distanceTop = glm::dot(top - planeTransform->Position, planeNormal);
        const float distanceBottom = glm::dot(bottom - planeTransform->Position, planeNormal);

        CollisionPoints points{};

        // Check for collision with the top point of the capsule
        if (glm::abs(distanceTop) < capsuleRadius)
        {
            points.ADeep = top - planeNormal * capsuleRadius;
            points.BDeep = top - planeNormal * glm::abs(distanceTop);
            points.Normal = -planeNormal;
            points.Depth = capsuleRadius - glm::abs(distanceTop);
            points.HasCollision = true;
        }
        // Check for collision with the bottom point of the capsule
        else if (glm::abs(distanceBottom) < capsuleRadius)
        {
            points.ADeep = bottom - planeNormal * capsuleRadius;
            points.BDeep = bottom - planeNormal * glm::abs(distanceBottom);
            points.Normal = -planeNormal;
            points.Depth = capsuleRadius - glm::abs(distanceBottom);
            points.HasCollision = true;
        }
    
        return points;
    }

    CollisionPoints TestSphereToCapsule(const Collider* sphereCollider, const Transform* sphereTransform,
                                    const Collider* capsuleCollider, const Transform* capsuleTransform)
    {
        const auto capsule = dynamic_cast<const CapsuleCollider*>(capsuleCollider);
        const auto sphere = dynamic_cast<const SphereCollider*>(sphereCollider);

        const glm::vec3 capsuleCenter = capsuleTransform->Position + capsule->Center;
        const float capsuleRadius = capsule->Radius;
        const float capsuleHeight = capsule->Height;
    
        const glm::vec3 top = capsuleCenter + glm::vec3(0.f, capsuleHeight / 2.f, 0.f);
        const glm::vec3 bottom = capsuleCenter - glm::vec3(0.f, capsuleHeight / 2.f, 0.f);

        const glm::vec3 sphereCenter = sphereTransform->Position + sphere->Center;
        const float sphereRadius = sphere->Radius;

        // Calculate the vector from bottom to top of the capsule
        const glm::vec3 capsuleSegment = top - bottom;

        // Project the sphere center onto the capsule segment
        const float t = glm::dot(sphereCenter - bottom, capsuleSegment) / glm::dot(capsuleSegment, capsuleSegment);
        const glm::vec3 closestPoint = bottom + glm::clamp(t, 0.f, 1.f) * capsuleSegment;

        // Calculate the distance between the closest point and the sphere
        const float distance = glm::distance(closestPoint, sphereCenter);

        const float depth = capsuleRadius + sphereRadius - distance;
        const glm::vec3 normal = glm::normalize(sphereCenter - closestPoint);
    
        CollisionPoints points{};
        if (distance < capsuleRadius + sphereRadius)
        {
            points.ADeep = closestPoint;
            points.BDeep = sphereCenter - normal * sphereRadius;
            points.Normal = normal;
            points.Depth = depth;
            points.HasCollision = true;
        }
    
        return points;
    }
}
