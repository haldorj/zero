#pragma once

#include <cstdint>
#include <memory>
#include <Model/Model.h>
#include <Model/Vulkan/VulkanModel.h>
#include <Physics/Physics.h>
#include <Physics/Collision/Collider.h>
#include <Scene/Transform.h>

namespace Zero
{
    class GameObject
    {
    public:
        using IdType = uint64_t;

        GameObject() = default;
        // Delete copy constructor and assignment operator
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

        std::shared_ptr<Collider> GetCollider() { return m_Collider; }
        void SetCollider(const std::shared_ptr<Collider>& collider) { m_Collider = collider; }

        RigidBody& GetRigidBody() { return m_RigidBody; }
        void SetModel(const std::shared_ptr<Model>& model) { m_Model = model; }

        std::shared_ptr<Model> GetModel() const { return m_Model; }

        void UpdatePlayer(float deltaTime);
        
        bool EnableGravity{false};
        bool EnableCollision{false};
        bool IsJumping = false;

    private:
        GameObject(IdType objectID) : m_ObjectID(objectID)
        {
        }

        IdType m_ObjectID{};

        std::shared_ptr<Model> m_Model{};
        std::shared_ptr<Collider> m_Collider{nullptr};
        Transform m_Transform{};
        RigidBody m_RigidBody{};
    };
}
