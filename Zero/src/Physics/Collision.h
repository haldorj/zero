#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Zero {

	struct CollisionPoints
	{
		glm::vec3 A;		// Furthest point of object A Into B
		glm::vec3 B;		// Furthest point of object B Into A
		glm::vec3 Normal;	// Normal of the collision
		float Depth;
		bool HasCollision;
	};

	struct Transform;
	struct SphereCollider;
	struct PlaneCollider;

	struct Collider
	{
		virtual CollisionPoints TestCollision(
			const Transform* transform,
			const Collider* collider,
			const Transform* otherTransform) const = 0;

		virtual CollisionPoints TestCollision(
			const Transform* transform,
			const SphereCollider* collider,
			const Transform* otherTransform) const = 0;

		virtual CollisionPoints TestCollision(
			const Transform* transform,
			const PlaneCollider* collider,
			const Transform* otherTransform) const = 0;

		virtual ~Collider() = default;
	};

	namespace Algo {
	
		CollisionPoints FindSphereToSphereCollision(
			const SphereCollider* sphereA, const Transform* transformA,
			const SphereCollider* sphereB, const Transform* transformB);
	
		CollisionPoints FindSphereToPlaneCollision(
			const SphereCollider* sphere, const Transform* transform,
			const PlaneCollider* plane, const Transform* otherTransform);
	
		CollisionPoints FindPlaneToSphereCollision(
			const PlaneCollider* plane, const Transform* transform,
			const SphereCollider* sphere, const Transform* otherTransform);
		
	}

	struct SphereCollider : Collider
	{
		SphereCollider() = default;
		SphereCollider(glm::vec3 center, float radius)
			: Center(center), Radius(radius) {}

		CollisionPoints TestCollision(
			const Transform* transform,
			const Collider* collider,
			const Transform* otherTransform) const override
		{
			return collider->TestCollision(otherTransform, this, transform);
		}

		CollisionPoints TestCollision(
			const Transform* transform,
			const SphereCollider* collider,
			const Transform* otherTransform) const override
		{
			printf("Testing Sphere to Sphere Collision\n");
			return Algo::FindSphereToSphereCollision(this, transform, collider, otherTransform);
		}

		CollisionPoints TestCollision(
			const Transform* transform,
			const PlaneCollider* collider,
			const Transform* otherTransform) const override
		{
			printf("Testing Sphere to Plane Collision\n");
			return Algo::FindSphereToPlaneCollision(this, transform, collider, otherTransform);
		}

		glm::vec3 Center{ 0.0f };
		float Radius{ 1.0f };
	};

	struct PlaneCollider : Collider
	{
		PlaneCollider() = default;
		PlaneCollider(glm::vec3 plane, float distance)
			: Plane(plane), Distance(distance) {}

		CollisionPoints TestCollision(
			const Transform* transform,
			const Collider* collider,
			const Transform* otherTransform) const override
		{
			printf("Testing Plane to Sphere Collision\n");
			return collider->TestCollision(otherTransform, this, transform);
		}

		CollisionPoints TestCollision(
			const Transform* transform,
			const SphereCollider* collider,
			const Transform* otherTransform) const override
		{
			printf("Testing Sphere to Plane Collision\n");
			return Algo::FindPlaneToSphereCollision(this, transform, collider, otherTransform);
		}

		CollisionPoints TestCollision(
			const Transform* transform,
			const PlaneCollider* collider,
			const Transform* otherTransform) const override
		{
			return {};
		}

		glm::vec3 Plane;
		float Distance;
	};

	class GameObject;

	struct Collision
	{
		std::shared_ptr<GameObject> A;
		std::shared_ptr<GameObject> B;
		CollisionPoints Points;
	};

}
