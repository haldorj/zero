#pragma once

#include <cstdint>
#include <string>
#include <glm/glm.hpp>
#include <memory>
#include <Model/Model.h>
#include <Model/Vulkan/VulkanModel.h>
#include <Physics/Physics.h>
#include <Physics/Collision.h>

namespace Zero {

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

		id_t GetID() const { return m_ObjectID; }
		
		Transform& GetTransform() { return m_Transform; }
		Collider* GetCollider() { return m_Collider; }
		void SetCollider(Collider* collider) { m_Collider = collider; }
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

		bool EnableGravity{false};

	private:
		GameObject(id_t objectID) : m_ObjectID(objectID) {}

		id_t m_ObjectID{};

		std::shared_ptr<Model> m_Model{nullptr};
		Transform m_Transform{};
		Collider* m_Collider{};
		Dynamics m_Dynamics{};
	};

}