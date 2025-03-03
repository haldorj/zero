#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace Zero {

	struct CollisionPoints
	{
		glm::vec3 ADeep;		// Furthest point of object A Into B
		glm::vec3 BDeep;		// Furthest point of object B Into A
		glm::vec3 Normal;		// Normal of the collision
		float Depth;			// Depth of the collision
		bool HasCollision;
	};

	enum ColliderType
	{
		Sphere,
		Plane,
		Capsule,
		Mesh
	};

	struct Collider
	{
		Collider() = default;
		virtual ~Collider() = default;

		virtual glm::vec3 FindFurthestPoint(const glm::vec3& direction) = 0;
		
		ColliderType Type;
	};

	class GameObject;
	
	struct Collision
	{
		std::shared_ptr<GameObject> ObjA;
		std::shared_ptr<GameObject> ObjB;
		CollisionPoints Points;
	};
}
