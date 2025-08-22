#pragma once

#include <cstdint>
#include <memory>
#include <Model/Model.h>
#include <Model/Vulkan/VulkanModel.h>
#include <Physics/Physics.h>
#include <Physics/Collision/Collider.h>
#include <Scene/Transform.h>
#include <Animation/Animator.h>

namespace Zero
{
    class GameObject
    {
    public:
        using IdType = uint32_t;

        GameObject() = default;
        // Delete copy constructor
        // to prevent duplicating/copying of GameObjects
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        // Enable move constructor and assignment operator
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        static GameObject Create();
        void Destroy() const;

        IdType GetID() const { return m_ObjectID; }

        Transform& GetTransform() { return m_Transform; }
        RigidBody& GetRigidBody() { return m_RigidBody; }

        Collider* GetCollider() { return m_Collider.get(); }
        void SetCollider(std::unique_ptr<Collider> collider) { m_Collider = std::move(collider); }

        void SetModel(const std::shared_ptr<Model>& model) { m_Model = model; }
        std::shared_ptr<Model> GetModel() const { return m_Model.lock(); }

        void SetAnimation(uint32_t index);
        Animator* GetAnimator() { return m_Animator.get(); }
		void SetAnimator(std::unique_ptr<Animator> animator) { m_Animator = std::move(animator); }

        void UpdatePlayer(float deltaTime);
        void UpdateAnimation(float deltaTime);
        
        bool EnableGravity{false};
        bool EnableCollision{false};
        bool IsJumping = false;
		bool IsGrounded = false;
		bool WasGrounded = false;

    private:
        GameObject(IdType objectID) : m_ObjectID(objectID) {}

        IdType m_ObjectID{};

        std::weak_ptr<Model> m_Model{};
        std::unique_ptr<Animator> m_Animator{};

        std::unique_ptr<Collider> m_Collider{};
        Transform m_Transform{};
        RigidBody m_RigidBody{};

		float m_LandingTimer{ 0 };
    };
}
