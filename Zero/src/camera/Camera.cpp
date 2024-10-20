#include "Camera.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

namespace Zero
{
    void Camera::Update()
    {
        const glm::mat4 cameraRotation = GetRotationMatrix();
        m_Position += glm::vec3(cameraRotation * glm::vec4(m_Velocity * 0.5f, 0.f));
    }

    void Camera::ProcessInput(GLFWwindow* window, float deltaTime)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            m_Velocity.z = -m_MovementSpeed * deltaTime;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            m_Velocity.z = m_MovementSpeed * deltaTime;
        }
        else
        {
            m_Velocity.z = 0.f;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            m_Velocity.x = -m_MovementSpeed * deltaTime;
        }
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            m_Velocity.x = m_MovementSpeed * deltaTime;
        }
        else
        {
            m_Velocity.x = 0.f;
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            m_Velocity.y = m_MovementSpeed * deltaTime;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        {
            m_Velocity.y = -m_MovementSpeed * deltaTime;
        }
        else
        {
            m_Velocity.y = 0.f;
        }

        // Mouse input
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        static double lastMouseX = mouseX, lastMouseY = mouseY;
        const double deltaX = mouseX - lastMouseX;
        const double deltaY = mouseY - lastMouseY;
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        // Calculate the new rotation for the GameObject
        m_Pitch = static_cast<float>(deltaTime * (deltaY * m_RotationSpeed));
        m_Yaw = static_cast<float>(deltaTime * (deltaX * m_RotationSpeed));
    }
    
    glm::mat4 Camera::GetViewMatrix()
    {
        const glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), m_Position);
        const glm::mat4 cameraRotation = GetRotationMatrix();
        return glm::inverse(cameraTranslation * cameraRotation);
    }

    glm::mat4 Camera::GetRotationMatrix()
    {
        const glm::quat pitchRotation = glm::angleAxis(m_Pitch, glm::vec3 { 1.f, 0.f, 0.f });
        const glm::quat yawRotation = glm::angleAxis(m_Yaw, glm::vec3 { 0.f, -1.f, 0.f });

        return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
    }
} // namespace Zero
