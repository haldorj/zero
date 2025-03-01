#pragma once

#include <glm/glm.hpp>
#include <memory>

#include <Scene/Transform.h>
#include <Physics/Solvers/Solver.h>

namespace Zero {

	class GameObject;

	class PhysicsWorld
	{
	public:
		PhysicsWorld() = default;
		~PhysicsWorld() = default;

		void Init();
		void Step(float dt, const std::vector<std::shared_ptr<GameObject>>& gameObjects) const;
		void ResolveCollisions(float dt, const std::vector<std::shared_ptr<GameObject>>& gameObjects) const;

	private:
		std::vector<std::shared_ptr<Solver>> m_Solvers;
	};

	struct RigidBody
	{
		void AddForce(const glm::vec3& force) { Force += force; }
		void AddImpulse(const glm::vec3& impulse) { Velocity += impulse / Mass; }

		glm::vec3 Velocity{ 0.0f };
		glm::vec3 Force{ 0.0f };
		float Mass{ 1.0f };
		float InvMass{ 0.0f / Mass };
		float Restitution{ 1.0f };
	};

}