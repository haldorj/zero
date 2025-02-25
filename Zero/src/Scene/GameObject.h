#pragma once

#include <cstdint>
#include <memory>
#include <Model/Model.h>
#include <Model/Vulkan/VulkanModel.h>
#include <Physics/Physics.h>
#include <Physics/Collision.h>

namespace Zero {

	class GameObject
	{
	public:
		using IdT = uint64_t;

		GameObject() = default;
		
		static GameObject Create() 
		{ 
			static IdT currentID = 0;
			return {currentID++};
		}

		IdT GetID() const { return m_ObjectID; }
		
		Transform& GetTransform() { return m_Transform; }
		
		std::shared_ptr<Collider> GetCollider() { return m_Collider; }
		void SetCollider(const std::shared_ptr<Collider>& collider) { m_Collider = collider; }
		
		Dynamics& GetDynamics() { return m_Dynamics; }
		void SetModel(const std::shared_ptr<Model>& model) { m_Model = model; }
		
		std::shared_ptr<Model> GetModel() const { return m_Model; }

		// Delete copy constructor and assignment operator
		// to prevent duplicating/copying of GameObjects
		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		// Enable move constructor and assignment operator
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		void Destroy();

		bool EnableGravity{false};

	private:
		GameObject(IdT objectID) : m_ObjectID(objectID) {}

		IdT m_ObjectID{};

		std::shared_ptr<Model> m_Model{};
		std::shared_ptr<Collider> m_Collider{};
		Transform m_Transform{};
		Dynamics m_Dynamics{};
	};

}