#include "EditorCamera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Application.h"

namespace Zero
{
    void EditorCamera::Update(float deltaTime)
    {
        const glm::mat4 cameraRotation = GetRotationMatrix();
        m_Transform.Position += glm::vec3(cameraRotation * glm::vec4(m_Direction * m_MovementSpeed * deltaTime, 0.f));
    }

    void EditorCamera::ProcessInput(GLFWwindow* window, float deltaTime)
    {
        if (!Application::Get().IsEditorMode())
            return;
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            m_Direction.z = -1;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            m_Direction.z = 1;
        }
        else
        {
            m_Direction.z = 0.f;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            m_Direction.x = -1;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            m_Direction.x = 1;
        }
        else
        {
            m_Direction.x = 0.f;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            m_Direction.y = 1;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            m_Direction.y = -1;
        }
        else
        {
            m_Direction.y = 0.f;
        }

        if (glm::length(m_Direction) > 0)
		{
			m_Direction = glm::normalize(m_Direction);
		}

        // Mouse input
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        static double lastMouseX = mouseX, lastMouseY = mouseY;
        const double deltaX = mouseX - lastMouseX;
        const double deltaY = mouseY - lastMouseY;
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        // Calculate the new rotation for the GameObject
        m_Pitch -= static_cast<float>(0.016 * (deltaY * m_RotationSpeed));
        m_Yaw += static_cast<float>(0.016 * (deltaX * m_RotationSpeed));

        if (m_Pitch > glm::radians(89.0f))
            m_Pitch = glm::radians(89.0f);
        if (m_Pitch < glm::radians(-89.0f))
            m_Pitch = glm::radians(-89.0f);

        if (m_Yaw > glm::radians(360.0f))
            m_Yaw -= glm::radians(360.0f);
        if (m_Yaw < 0)
            m_Yaw += glm::radians(360.0f);
    }
    
    glm::mat4 EditorCamera::GetViewMatrix() const
    {
        const glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), m_Transform.Position);
        const glm::mat4 cameraRotation = GetRotationMatrix();
        return glm::inverse(cameraTranslation * cameraRotation);
    }

    glm::mat4 EditorCamera::GetRotationMatrix() const
    {
        const glm::quat pitchRotation = glm::angleAxis(m_Pitch, glm::vec3 { 1.f, 0.f, 0.f });
        const glm::quat yawRotation = glm::angleAxis(m_Yaw, glm::vec3 { 0.f, -1.f, 0.f });

        return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
    }

    glm::vec3 EditorCamera::GetForwardVector() const
    {
        const glm::mat4 rotation = GetRotationMatrix();

		return glm::normalize(glm::vec3(rotation * glm::vec4(0.f, 0.f, -1.f, 0.f)));
    }
} // namespace Zero
