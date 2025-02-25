#include "Physics.h"

#include <memory>
#include <Core/core.h>
#include <Scene/GameObject.h>

#include <Physics/Collision.h>

namespace Zero {
	void PhysicsWorld::Init()
	{
		const auto impulseSolver = std::make_shared<ImpulseSolver>();
		m_Solvers.emplace_back(impulseSolver);
	}

	void PhysicsWorld::Step(const float dt, std::vector<std::shared_ptr<GameObject>>& gameObjects) const
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

	void PhysicsWorld::ResolveCollisions(const float dt, std::vector<std::shared_ptr<GameObject>>& gameObjects) const
	{
		std::vector<Collision> collisions;
		for (auto& obj : gameObjects)
		{
			for (const auto& solver : m_Solvers)
			{
				solver->Solve(collisions, dt);
			}

			for (auto& other : gameObjects)
			{
				if (!obj)
				{
					break;
				}
				if (obj == other)
				{
					break;
				}
				if (!obj->GetCollider() || !other->GetCollider())
				{
					continue;
				}

				CollisionPoints collisionPoints = obj->GetCollider()->TestCollision(&obj->GetTransform(),
					other->GetCollider().get(), &other->GetTransform());
				
				if (collisionPoints.HasCollision)
				{
					collisions.emplace_back(obj, other, collisionPoints);
				}
			}
		}
	}

}