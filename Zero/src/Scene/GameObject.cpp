#include "GameObject.h"
#include "Application.h"
#include "glm/detail/type_quat.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Zero
{
    GameObject GameObject::Create()
    {
        static IdType currentID = 0;
        return {currentID++};
    }

    void GameObject::Destroy() const
    {
        if (Application::Get().GetRendererType() == RendererAPI::OpenGL)
        {
            return;
        }

        if (m_Model)
        {
            m_Model->DestroyModel();
        }
    }

    void GameObject::SetAnimation(Animation* animation)
    {
        m_Animation = animation;
        m_Animator = std::make_shared<Animator>(m_Animation);
    }

    void GameObject::UpdatePlayer(float deltaTime)
    {
        if (Application::Get().IsEditorMode())
            return;

        GLFWwindow* window = Application::Get().GetWindow();
        const Camera* camera = &Application::Get().GetActiveCamera();

        glm::vec3 direction{0};
        constexpr float moveSpeed = 20;
        float acceleration = 300.0f;
        float rotationSpeed = 15.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            direction.x -= camera->GetTransform().GetForwardVector().x;
            direction.z -= camera->GetTransform().GetForwardVector().z;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            direction.x += camera->GetTransform().GetForwardVector().x;
            direction.z += camera->GetTransform().GetForwardVector().z;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            direction.x += camera->GetTransform().GetRightVector().x;
            direction.z += camera->GetTransform().GetRightVector().z;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            direction.x -= camera->GetTransform().GetRightVector().x;
            direction.z -= camera->GetTransform().GetRightVector().z;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            acceleration *= 2.0f;
            rotationSpeed *= 2.0f;
        }
        
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (!IsJumping)
            {
                constexpr float jumpForce = 100.0f;
                if (glm::length(direction) > 0.0001f)
                {
                    const glm::vec3 jumpDirection = glm::normalize(glm::vec3(0, 1, 0) + glm::normalize(direction));
                    m_RigidBody.AddImpulse(jumpDirection * jumpForce);
                    IsJumping = true;
                }
                else
                {
                    m_RigidBody.AddImpulse(glm::vec3(0, 1, 0) * jumpForce);
                    IsJumping = true;
                }
                

            }
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        {
            IsJumping = false;
        }

        if (glm::length(direction) > 0.0001f)
        {
            direction = glm::normalize(direction);
            const glm::quat targetRotation = glm::quatLookAt(-direction, glm::vec3(0, 1, 0));
            // Create quaternion from direction

            // Interpolate between current and target rotation
            m_Transform.RotationQuat = glm::slerp(m_Transform.RotationQuat, targetRotation, rotationSpeed * deltaTime);
            m_Transform.Rotation = glm::eulerAngles(m_Transform.RotationQuat);
        }

        m_RigidBody.Velocity.x += direction.x * acceleration * deltaTime;
        m_RigidBody.Velocity.z += direction.z * acceleration * deltaTime;
        
        if (length(m_RigidBody.Velocity) >= moveSpeed)
        {
            const glm::vec3 normalizedVelocity = normalize(m_RigidBody.Velocity);
            m_RigidBody.Velocity.x = normalizedVelocity.x * moveSpeed;
            m_RigidBody.Velocity.z = normalizedVelocity.z * moveSpeed;
        }
        if (direction == glm::vec3(0))
        {
            // Decelerate
            const glm::vec3 velocityDirection = glm::normalize(m_RigidBody.Velocity);
            m_RigidBody.Velocity.x -= velocityDirection.x * acceleration * deltaTime;
            m_RigidBody.Velocity.z -= velocityDirection.z * acceleration * deltaTime;
        }
    }

    void GameObject::UpdateAnimation(float deltaTime)
    {
        if (m_Animator && m_Animation)
        {
            m_Animator->UpdateAnimation(deltaTime);
        }
            
    }
}
