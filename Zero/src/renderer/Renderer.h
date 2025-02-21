#pragma once

#include "glm/glm.hpp"
#include <span>
#include <Scene/GameObject.h>

namespace Zero
{
    enum class Topology
    {
        None,
        Lines,
        Triangles,
    };
}

// Renderer interface
namespace Zero
{
    class Renderer
    {
    public:
        virtual ~Renderer() = default;

        virtual void Init() = 0;

        virtual void InitImGui() = 0;

        virtual void Shutdown() = 0;

        virtual void SetClearColor(glm::vec4 clearColor) = 0;

        virtual void Draw(std::vector<std::shared_ptr<GameObject>>& gameObjects, Topology) = 0;

        // bool VSync = true;
    };
}
