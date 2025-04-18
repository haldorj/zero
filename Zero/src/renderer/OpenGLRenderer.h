#pragma once

#include "OpenGL/EBO.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/VAO.h"

#include "renderer/Renderer.h"
#include "renderer/OpenGL/OpenGLTexture.h"
#include <Model/OpenGL/OpenGLMesh.h>
#include <Model/OpenGL/OpenGLModel.h>
#include <Scene/GameObject.h>

#include "OpenGL/OpenGLUtils.h"

namespace Zero {

    class OpenGLRenderer : public Renderer
    {
    public:
        void Init() override;
        void InitImGui() override;
        void Shutdown() override;
        void SetClearColor(glm::vec4 clearColor) override { m_ClearColor = clearColor; }
        void Draw(Scene* scene) override;

        void SetUniformValues(OpenGLShader* shader, Scene* scene);

    private:
        void InitShaders();

        glm::vec4 m_ClearColor = {};
        std::shared_ptr<OpenGLShader> m_ShaderProgram = nullptr;

        UniformDirectionalLight m_UniformDirectionalLight{};

        size_t m_PointLightCount{};
        std::array<UniformPointLight, MAX_POINTLIGHTS> m_UniformPointLights{};

        int m_Width{};
        int m_Height{};
    };

} 
