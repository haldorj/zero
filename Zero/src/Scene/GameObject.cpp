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
    }

    void GameObject::SetAnimation(uint32_t index)
    {
		// Update animator and play the new animation.
		// Animations are picked by index, so ensure the index is valid.
        GetAnimator()->PlayAnimation(index);
    }

    void GameObject::UpdatePlayer(float deltaTime)
    {
        if (Application::Get().IsEditorMode())
            return;

        GLFWwindow* window = Application::Get().GetWindow();
        const Camera* camera = &Application::Get().GetActiveCamera();

        glm::vec3 direction{0};
        constexpr float moveSpeed = 6;
        float acceleration = 300.0f;
        float rotationSpeed = 15.0f;

		IsGrounded = m_Transform.Position.y <= 1.f; // Simple ground check
        bool justLanded = IsGrounded && !WasGrounded;
        bool justJumped = !IsGrounded && WasGrounded;
        float verticalVelocity = m_RigidBody.Velocity.y;

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
            if (!IsJumping && IsGrounded)
            {
                constexpr float jumpForce = 50.0f;
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
            // Create quaternion from direction
            direction = glm::normalize(direction);
            const glm::quat targetRotation = glm::quatLookAt(-direction, glm::vec3(0, 1, 0));

            // Interpolate between current and target rotation
            const glm::quat& RotationQuat = glm::slerp(glm::quat(m_Transform.Rotation), targetRotation, rotationSpeed * deltaTime);
            m_Transform.Rotation = glm::eulerAngles(RotationQuat);
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

        if (justLanded && !justJumped)
        {
            m_LandingTimer = 0.5f; // time in seconds to show landing animation
        }

        if (!IsGrounded)
        {
            if (verticalVelocity > 0.1f)
            {
                // Jumping upward
                if (GetAnimator()->GetCurrentAnimationIndex() != 0)
                    GetAnimator()->PlayAnimation(0, false);
            }
            else if (verticalVelocity < -0.1f)
            {
				// Falling down
                if (GetAnimator()->GetCurrentAnimationIndex() != 1)
                    GetAnimator()->PlayAnimation(1, false);
            }
        }
        else
        {
            float threshold = 2.f;
            if (m_LandingTimer > 0.0f)
            {
                m_LandingTimer -= deltaTime;
                if (GetAnimator()->GetCurrentAnimationIndex() != 4)
                    GetAnimator()->PlayAnimation(4);
                // Do not switch to run/idle yet — let landing animation play
            }
            else if (std::abs(m_RigidBody.Velocity.x) > threshold ||
                std::abs(m_RigidBody.Velocity.z) > threshold)
            {
                // Run
                if (GetAnimator()->GetCurrentAnimationIndex() != 7)
                    GetAnimator()->PlayAnimation(7);
            }
            else
            {
                // Idle
                if (GetAnimator()->GetCurrentAnimationIndex() != 2)
                    GetAnimator()->PlayAnimation(2);
            }
        }
		WasGrounded = IsGrounded;
    }

    void GameObject::UpdateAnimation(float deltaTime)
    {
        if (GetAnimator())
        {
            GetAnimator()->UpdateAnimation(deltaTime);
        }
            
    }
}
