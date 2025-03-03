#pragma once

#include "OpenGL/EBO.h"
#include "OpenGL/OpenGLShader.h"
#include "OpenGL/VAO.h"

#include "renderer/Renderer.h"
#include "renderer/OpenGL/OpenGLTexture.h"
#include <Model/OpenGL/OpenGLMesh.h>
#include <Model/OpenGL/OpenGLModel.h>
#include <Scene/GameObject.h>

namespace Zero {

    class OpenGLRenderer : public Renderer
    {
    public:
        void Init() override;
        void InitImGui() override;
        void Shutdown() override;
        void SetClearColor(glm::vec4 clearColor) override { m_ClearColor = clearColor; }
        void Draw(std::vector<std::shared_ptr<GameObject>>& gameObjects, Topology) override;

    private:
        void InitShaders();

        glm::vec4 m_ClearColor = {};
        std::shared_ptr<OpenGLShader> m_ShaderProgram;
        std::shared_ptr<OpenGLShader> m_DebugShader;

        std::array<std::shared_ptr<OpenGLShader>, 2> m_ShaderPrograms;

        int m_Width{};
        int m_Height{};
    };

} 
