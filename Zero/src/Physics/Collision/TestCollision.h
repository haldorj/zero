#pragma once
#include "Collider.h"
#include "Scene/Transform.h"

namespace Zero
{
    CollisionPoints TestSphereToSphere(
        const Collider* sphereColliderA, const Transform* sphereTransformA,
        const Collider* sphereColliderB, const Transform* sphereTransformB);

    CollisionPoints TestSphereToPlane(
        const Collider* sphereCollider, const Transform* sphereTransform,
        const Collider* planeCollider, const Transform* planeTransform);

    CollisionPoints TestCapsuleToCapsule(
        const Collider* capsuleColliderA, const Transform* capsuleTransformA,
        const Collider* capsuleColliderB, const Transform* capsuleTransformB);

    CollisionPoints TestPlaneToCapsule(
        const Collider* planeCollider, const Transform* planeTransform,
        const Collider* capsuleCollider, const Transform* capsuleTransform);

    CollisionPoints TestSphereToCapsule(
        const Collider* sphereCollider, const Transform* sphereTransform,
        const Collider* capsuleCollider, const Transform* capsuleTransform);

    using FindContactFunc = CollisionPoints(*)(
        const Collider*, const Transform*,
        const Collider*, const Transform*);

    inline CollisionPoints TestCollision(
        const Collider* a, const Transform* at,
        const Collider* b, const Transform* bt)
    {
        static FindContactFunc tests[3][3] =
        {
            // Sphere                  Plane                   Capsule
            {TestSphereToSphere,    TestSphereToPlane,      TestSphereToCapsule}, // Sphere
            {nullptr,               nullptr,                TestPlaneToCapsule},             // Plane
            {nullptr,               nullptr,                TestCapsuleToCapsule} // Capsule
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
            std::swap(points.ADeep, points.BDeep);
            points.Normal = -points.Normal;
        }

        return points;
    }
}
