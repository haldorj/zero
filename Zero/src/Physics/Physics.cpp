#include "Physics.h"

#include <memory>
#include <Core/core.h>
#include <Scene/GameObject.h>

#include <Physics/Collision/Collider.h>

#include "Collision/TestCollision.h"

namespace Zero {
	void PhysicsWorld::Init()
	{
		m_Solvers.emplace_back(std::make_shared<TestSolver>());
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

			obj->GetDynamics().Force = obj->GetDynamics().Mass * GRAVITY;
			obj->GetDynamics().Velocity += obj->GetDynamics().Force / obj->GetDynamics().Mass * dt;
			obj->GetTransform().Location += obj->GetDynamics().Velocity * dt;

			obj->GetDynamics().Force = glm::vec3(0.0f);
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
