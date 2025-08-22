#pragma once
#include "Camera.h"
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"

namespace Zero 
{
    class PerspectiveCamera : Camera
    {
    public:
        void Update(float deltaTime, const glm::vec3& targetPosition);
        void ProcessInput(GLFWwindow* window, float deltaTime);
        glm::mat4 GetViewMatrix() const override;
        glm::mat4 GetRotationMatrix() const override;

        glm::vec3 GetPosition() const override { return m_Transform.Position; }
        void SetPosition(const glm::vec3& position) { m_Transform.Position = position; }
        Transform GetTransform() const override { return m_Transform; }
        
        glm::vec3 GetForwardVector() const;
        glm::vec3 GetDirection() const { return m_Direction; }

        float GetFOV() const override { return m_FOV; }
        void SetFOV(float fov) { m_FOV = fov; }

    private:
        Transform m_Transform = {};

        glm::vec3 m_Direction = {};
        glm::vec3 m_TargetPosition{ 0.0f };

        float m_FOV = 70.f;
        float m_RotationSpeed = 0.4f;

        float m_Pitch{ 0.f };
        float m_Yaw{ 180.f };

        float m_DistanceFromTarget = 7.0f;
    };
}
