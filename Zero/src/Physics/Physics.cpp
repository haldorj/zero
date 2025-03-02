#include "Physics.h"

#include <memory>
#include <Core/core.h>
#include <Scene/GameObject.h>

#include <Physics/Collision/Collider.h>

#include "Collision/TestCollision.h"

namespace Zero {
	void PhysicsWorld::Init()
	{
		m_Solvers.emplace_back(std::make_shared<PositionSolver>());
		m_Solvers.emplace_back(std::make_shared<ImpulseSolver>());
	}

	void PhysicsWorld::Step(const float dt, const std::vector<std::shared_ptr<GameObject>>& gameObjects) const
	{
		ResolveCollisions(dt, gameObjects);
		for (const auto& obj : gameObjects)
		{
			if (!obj->EnableGravity) 
			{
				continue;
			}

			obj->GetRigidBody().Force = obj->GetRigidBody().Mass * GRAVITY;
			obj->GetRigidBody().Velocity += obj->GetRigidBody().Force / obj->GetRigidBody().Mass * dt;
			obj->GetTransform().Position += obj->GetRigidBody().Velocity * dt;

			obj->GetRigidBody().Force = glm::vec3(0.0f);
		}
	}

	void PhysicsWorld::ResolveCollisions(const float dt, const std::vector<std::shared_ptr<GameObject>>& gameObjects) const
	{
		std::vector<Collision> collisions;

		for (const auto& a : gameObjects)
		{
			for (const auto& b : gameObjects) 
			{
				if (a == b)
				{
					break;
				}
				
				if (!a->GetCollider() || !b->GetCollider())
				{
					continue;
				}

				if (!a->EnableCollision || !b->EnableCollision)
				{
					continue;
				}
				
				CollisionPoints points = TestCollision(
					a->GetCollider().get(), &a->GetTransform(),
					b->GetCollider().get(), &b->GetTransform()
				);

				if (points.HasCollision)
				{
					collisions.emplace_back(a, b, points);
				}
			}
		}

		for (const auto& solver : m_Solvers)
		{
			solver->Solve(collisions, dt);
		}
	}

}
