#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <memory>
#include <Model/Model.h>
#include <Model/Vulkan/VulkanModel.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace Zero {

	struct Transform
	{
		glm::vec3 Location{0.0f};
		glm::vec3 Rotation{0.0f};
		glm::vec3 Scale{1.0f};

		glm::mat4 GetMatrix() const
		{
			return glm::translate(glm::mat4(1.0f), Location) *
				glm::rotate(glm::mat4(1.0f), Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
				glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct Dynamics
	{
		void AddForce(const glm::vec3& force) { Force += force; }
		void AddImpulse(const glm::vec3& impulse) { Velocity += impulse / Mass; }

		glm::vec3 Velocity{0.0f};
		glm::vec3 Force{0.0f};
		float Mass{1.0f};
	};

	class GameObject
	{
	public:
		using id_t = uint64_t;

		GameObject() = default;

		static GameObject Create() 
		{ 
			static id_t currentID = 0;
			return GameObject(currentID++);
		}

		void Update(float dt);

		id_t GetID() const { return m_ObjectID; }
		
		Transform& GetTransform() { return m_Transform; }
		Dynamics& GetDynamics() { return m_Dynamics; }
		void SetModel(std::shared_ptr<Model> model) { m_Model = model; }
		std::shared_ptr<Model> GetModel() const { return m_Model; }

		// Delete copy constructor and assignment operator
		// to prevent duplicating/copying of GameObjects
		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		// Enable move constructor and assignment operator
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		bool EnablePhysics{false};

	private:
		GameObject(id_t objectID) : m_ObjectID(objectID) {}

		id_t m_ObjectID{};

		std::shared_ptr<Model> m_Model{nullptr};
		Transform m_Transform{};
		Dynamics m_Dynamics{};
	};

}