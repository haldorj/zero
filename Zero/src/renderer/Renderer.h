#pragma once

#include "glm/glm.hpp"
#include <span>
#include <Model/Vertex.h>

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

        virtual void InitObjects(std::vector<std::string>& paths) = 0;

        virtual void Shutdown() = 0;

        virtual void SetClearColor(glm::vec4 clearColor) = 0;

        virtual void Draw(Topology) = 0;
    };
}
