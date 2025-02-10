#pragma once

#include "EnumModes/RendererMode.hpp"
#include "OpenGL/EBO.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/VAO.h"

#include "renderer/Renderer.h"
#include "renderer/OpenGL/OpenGLTexture.h"
#include <Mesh/Mesh.h>

namespace Zero
{
    class OpenGLRenderer : public RendererBase
    {
    public:
        void Init() override;
        void InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices) override;
        void Shutdown() override;
        void SetClearColor(glm::vec4 clearColor) override { _clearColor = clearColor; }
        void Draw(Topology) override;

    private:
        void InitShaders();

        Mesh* Pyramid{};

        glm::vec4 _clearColor = {};
        std::unique_ptr<OpenGLShader> shaderProgram;
    };
} // namespace Zero
