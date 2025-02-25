#include "Collision.h"

#include "Scene/Transform.h"


namespace Zero
{
    CollisionPoints Algo::FindSphereToSphereCollision(const SphereCollider* sphereA, const Transform* transformA,
                                                      const SphereCollider* sphereB, const Transform* transformB)
    {
        // const glm::vec3 centerA = transformA->Location + sphereA->Center;
        // const glm::vec3 centerB = transformB->Location + sphereB->Center;
        // const float radiusA = sphereA->Radius;
        // const float radiusB = sphereB->Radius;
        //
        // const glm::vec3 diff = centerB - centerA;
        // const float distance = glm::length(diff);
        // const float radiusSum = radiusA + radiusB;
        //
        // CollisionPoints points{};
        // if (distance < radiusSum)
        // {
        //     // Furthest point of object A Into B
        //     points.A = centerA + (diff / distance) * radiusA;
        //     // Furthest point of object B Into A
        //     points.B = centerB - (diff / distance) * radiusB;
        //     points.Normal = glm::normalize(diff);
        //     // Depth of the collision
        //     points.Depth = radiusSum - distance;
        //     points.HasCollision = true;
        // }
        // else
        // {
        //     points.HasCollision = false;
        // }

        return {};
    }

    CollisionPoints Algo::FindSphereToPlaneCollision(const SphereCollider* sphere, const Transform* transform,
                                                     const PlaneCollider* plane, const Transform* otherTransform)
    {
        const glm::vec3 sphereCenter = transform->Location;
        const glm::vec3 planeNormal = glm::normalize(plane->Plane);
        
        glm::vec3 pointOnPlane = otherTransform->Location;
        
        float distance = glm::dot(sphereCenter - pointOnPlane, planeNormal);

        CollisionPoints points{};
        if (std::abs(distance) < sphere->Radius)  // Correct condition check
        {
            points.A = sphereCenter - planeNormal * std::abs(distance);
            points.B = points.A - planeNormal * (sphere->Radius - std::abs(distance));
            points.Normal = planeNormal;
            points.Depth = sphere->Radius - std::abs(distance);
            points.HasCollision = true;
        }
        else
        {
            points.HasCollision = false;
        }

        return points;
    }


    CollisionPoints Algo::FindPlaneToSphereCollision(const PlaneCollider* plane, const Transform* transform,
                                                     const SphereCollider* sphere, const Transform* otherTransform)
    {
        return FindSphereToPlaneCollision(sphere, otherTransform, plane, transform);
    }
}
