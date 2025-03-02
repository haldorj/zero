#pragma once
#include "glm/fwd.hpp"
#include "Scene/Transform.h"

namespace Zero
{
    class Camera
    {
    public:
        Camera() = default;
        virtual ~Camera() = default;
        virtual glm::mat4 GetViewMatrix() const = 0;
        virtual glm::mat4 GetRotationMatrix() const = 0;

        virtual glm::vec3 GetPosition() const = 0;
        virtual float GetFOV() const = 0;

        virtual Transform GetTransform() const = 0;
    };
}

