#pragma once
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"

namespace Zero
{
    class Camera
    {
    public:
        void Update();
        void ProcessInput(GLFWwindow* window, float deltaTime);
        glm::mat4 GetViewMatrix();
        glm::mat4 GetRotationMatrix();

        glm::vec3 GetPosition() const { return m_Position; }
        void SetPosition(const glm::vec3& position) { m_Position = position; }

    private:
        // vertical rotation
        float m_Pitch{0.f};
        // horizontal rotation
        float m_Yaw{0.f};

        glm::vec3 m_Velocity = {};
        glm::vec3 m_Position = {};

        float m_MovementSpeed = 6.f;
        float m_RotationSpeed = 1.f;
    };
} // namespace Zero
