#pragma once

#include <glm/glm.hpp>
#include <memory>

#include <Scene/Transform.h>
#include <Physics/Solver.h>

namespace Zero {

	class GameObject;

	class PhysicsWorld
	{
	public:
		PhysicsWorld() = default;
		~PhysicsWorld() = default;

		void Step(float dt, std::vector<std::shared_ptr<GameObject>>& gameObjects);
		void ResolveCollisions(float dt, std::vector<std::shared_ptr<GameObject>>& gameObjects);

	private:
		std::vector<std::shared_ptr<Solver>> m_Solvers;
	};

	struct Dynamics
	{
		void AddForce(const glm::vec3& force) { Force += force; }
		void AddImpulse(const glm::vec3& impulse) { Velocity += impulse / Mass; }

		glm::vec3 Velocity{ 0.0f };
		glm::vec3 Force{ 0.0f };
		float Mass{ 1.0f };
	};

}