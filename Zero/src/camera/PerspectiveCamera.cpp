#include "PerspectiveCamera.h"

#include "EditorCamera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Zero
{
    void PerspectiveCamera::Update(float deltaTime, const glm::vec3& targetPosition)
    {
        m_TargetPosition = targetPosition + glm::vec3(0.0f, 1.0f, 0.0f);
        
        m_Transform.Position.x = m_TargetPosition.x + m_DistanceFromTarget * cos(m_Pitch) * sin(m_Yaw);
        m_Transform.Position.y = m_TargetPosition.y + m_DistanceFromTarget * sin(m_Pitch);
        m_Transform.Position.z = m_TargetPosition.z + m_DistanceFromTarget * cos(m_Pitch) * cos(m_Yaw);

        m_Transform.Rotation = {-m_Pitch, m_Yaw - glm::radians(180.0f), 0.0f};
    }

    void PerspectiveCamera::ProcessInput(GLFWwindow* window, const float deltaTime)
    {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            m_DistanceFromTarget -= 5 * deltaTime; // Zoom in
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            m_DistanceFromTarget += 5 * deltaTime; // Zoom out

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
        m_Yaw -= static_cast<float>(0.016 * (deltaX * m_RotationSpeed));

        if (m_Pitch > glm::radians(89.0f))
			m_Pitch = glm::radians(89.0f);
		if (m_Pitch < glm::radians(-89.0f))
            m_Pitch = glm::radians(-89.0f);

        if (m_Yaw > glm::radians(360.0f))
            m_Yaw -= glm::radians(360.0f);
        if (m_Yaw < 0)
            m_Yaw += glm::radians(360.0f);

        // Update the camera's position and rotation
		Update(deltaTime, m_TargetPosition);
    }

    glm::mat4 PerspectiveCamera::GetViewMatrix() const
    {
        return  glm::lookAt(m_Transform.Position, m_TargetPosition, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 PerspectiveCamera::GetRotationMatrix() const
    {
        const glm::quat pitchRotation = glm::angleAxis(m_Pitch, glm::vec3{1.f, 0.f, 0.f});
        const glm::quat yawRotation = glm::angleAxis(m_Yaw, glm::vec3{0.f, -1.f, 0.f});

        return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
    }

    glm::vec3 PerspectiveCamera::GetForwardVector() const
    {
        const glm::mat4 rotation = GetRotationMatrix();

        return glm::normalize(glm::vec3(rotation * glm::vec4(0.f, 0.f, -1.f, 0.f)));
    }
} // namespace Zero
