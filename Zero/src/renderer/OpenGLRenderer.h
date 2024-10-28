#pragma once

#include "OpenGL/EBO.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/VAO.h"

#include "renderer/Renderer.h"
#include "renderer/OpenGL/OpenGLTexture.h"

namespace Zero
{
    class OpenGLRenderer : public RendererBase
    {
    public:
        void Init() override;
        void InitObject(std::span<uint32_t> indices, std::span<Vertex> vertices) override;
        void Shutdown() override;
        void SetClearColor(glm::vec4 clearColor) override { _clearColor = clearColor; }
        void Draw() override;

    private:
        void InitShaders();

        glm::vec4 _clearColor = {};
        std::unique_ptr<OpenGLShader> shaderProgram;

        std::unique_ptr<VAO> VAO1;
        std::unique_ptr<VBO> VBO1;
        std::unique_ptr<EBO> EBO1;

        GLuint uniID = 0;
        std::shared_ptr<OpenGLTexture> texture1;
    };
} // namespace Zero
