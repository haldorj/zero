#pragma once
#include "Collider.h"

namespace Zero
{
    CollisionPoints TestSphereToSphere(
        const Collider* sphereColliderA, const Transform* sphereTransformA,
        const Collider* sphereColliderB, const Transform* sphereTransformB);

    CollisionPoints TestSphereToPlane(
        const Collider* sphereCollider, const Transform* sphereTransform,
        const Collider* planeCollider, const Transform* planeTransform);

    using FindContactFunc = CollisionPoints(*)(
        const Collider*, const Transform*, 
        const Collider*, const Transform*);

    inline CollisionPoints TestCollision(
        const Collider* a, const Transform* at, 
        const Collider* b, const Transform* bt)
    {
        static FindContactFunc tests[2][2] = 
        {
            // Sphere             Plane
            { TestSphereToSphere, TestSphereToPlane }, // Sphere
            { nullptr,            nullptr           }  // Plane
        };

        // If we are passed a Plane vs Sphere, swap the 
        // colliders, so it's a Sphere vs Plane
        const bool swap = b->Type < a->Type;

        if (swap)
        {
            std::swap(a, b);
            std::swap(at, bt);
        }

        // now we can dispatch the correct function
        if (!tests[a->Type][b->Type])
        {
            return {};
        }
        CollisionPoints points = tests[a->Type][b->Type](a, at, b, bt);

        // if we swapped the order of the colliders, to keep the
        // results consistent, we need to swap the points
        if (swap)
        {
            std::swap(points.A, points.B);
            points.Normal = -points.Normal;
        }

        return points;
    }
}
