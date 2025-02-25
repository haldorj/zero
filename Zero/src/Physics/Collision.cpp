#include "Collision.h"



namespace Zero
{
    CollisionPoints Algo::FindSphereToSphereCollision(const SphereCollider* sphereA, const Transform* transformA,
        const SphereCollider* sphereB, const Transform* transformB)
    {
        
        return {};
    }

    CollisionPoints Algo::FindSphereToPlaneCollision(const SphereCollider* sphere, const Transform* transform,
        const PlaneCollider* plane, const Transform* otherTransform)
    {
        return {};
    }

    CollisionPoints Algo::FindPlaneToSphereCollision(const PlaneCollider* plane, const Transform* transform,
        const SphereCollider* sphere, const Transform* otherTransform)
    {
        return {};
    }
}
