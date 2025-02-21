#pragma once
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"

namespace Zero
{
    class Camera
    {
    public:
        void Update(float deltaTime);
        void ProcessInput(GLFWwindow* window, float deltaTime);
        glm::mat4 GetViewMatrix();
        glm::mat4 GetRotationMatrix();

        glm::vec3 GetPosition() const { return m_Position; }
        void SetPosition(const glm::vec3& position) { m_Position = position; }

        glm::vec3 GetForwardVector();

        glm::vec3 GetDirection() const { return m_Direction; }

        float GetFOV() const { return m_FOV; }
        void SetFOV(float fov) { m_FOV = fov; }

    private:
        // vertical rotation
        float m_Pitch{0.f};
        // horizontal rotation
        float m_Yaw{180.f};

        glm::vec3 m_Direction = {};
        glm::vec3 m_Position = {};

        float m_FOV = 70.f;

        float m_MovementSpeed = 30.f;
        float m_RotationSpeed = 0.4f;
    };
} // namespace Zero
