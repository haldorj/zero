#pragma once

#include "EnumModes/RendererMode.hpp"
#include "OpenGL/EBO.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/VAO.h"

#include "renderer/Renderer.h"
#include "renderer/OpenGL/OpenGLTexture.h"
#include <Model/OpenGL/OpenGLMesh.h>
#include <Model/OpenGL/OpenGLModel.h>

namespace Zero
{
    class OpenGLRenderer : public RendererBase
    {
    public:
        void Init() override;
        void InitObjects(std::vector<std::string>& paths) override;
        void Shutdown() override;
        void SetClearColor(glm::vec4 clearColor) override { m_ClearColor = clearColor; }
        void Draw(Topology) override;

    private:
        void InitShaders();

        glm::vec4 m_ClearColor = {};
        std::shared_ptr<OpenGLShader> m_ShaderProgram;

        std::vector<std::shared_ptr<OpenGLModel>> m_Models;
    };
} // namespace Zero
