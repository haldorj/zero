#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Zero {

	struct CollisionPoints
	{
		glm::vec3 ADeep;		// Furthest point of object A Into B
		glm::vec3 BDeep;		// Furthest point of object B Into A
		glm::vec3 Normal;	// Normal of the collision
		float Depth;		// Depth of the collision
		bool HasCollision;
	};

	struct Transform;
	struct SphereCollider;
	struct PlaneCollider;

	enum ColliderType
	{
		Sphere,
		Plane
	};

	struct Collider
	{
		Collider() = default;
		virtual ~Collider() = default;

		virtual glm::vec3 FindFurthestPoint(const glm::vec3& direction) = 0;
		
		ColliderType Type;
	};

	struct SphereCollider : Collider
	{
		SphereCollider(const glm::vec3& center, const float radius)
			: Center(center), Radius(radius)
		{
			Type = ColliderType::Sphere;
		}

		glm::vec3 FindFurthestPoint(const glm::vec3& direction) override
		{
			return {};
		}
		
		glm::vec3 Center;
		float Radius;
	};

	struct PlaneCollider : Collider
	{
		PlaneCollider(const glm::vec3& normal, const float distance)
			: Normal(normal), Distance(distance)
		{
			Type = ColliderType::Plane;
		}

		glm::vec3 FindFurthestPoint(const glm::vec3& direction) override
		{
			return {};
		}
		
		glm::vec3 Normal;
		float Distance;
	};

	struct MeshCollider : Collider
	{
		MeshCollider() = default;

		glm::vec3 FindFurthestPoint(const glm::vec3& direction) override
		{
			glm::vec3 maxPoint{};
			float maxDistance = -FLT_MAX;

			for (const auto& vertex : Vertices)
			{
				const float distance = glm::dot(vertex, direction);
				if (distance > maxDistance)
				{
					maxDistance = distance;
					maxPoint = vertex;
				}
			}
			
			return maxPoint;
		}
		
		std::vector <glm::vec3> Vertices;
	};

	class GameObject;
	
	struct Collision
	{
		std::shared_ptr<GameObject> ObjA;
		std::shared_ptr<GameObject> ObjB;
		CollisionPoints Points;
	};
}
