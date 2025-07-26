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
#include <Scene/Skybox/Skybox.h>
#include <Scene/Shadowmap/OpenGLShadowmap.h>

namespace Zero {

    class OpenGLRenderer : public Renderer
    {
    public:
        void Init() override;
        void InitImGui() override;
        void Shutdown() override;
        void SetClearColor(glm::vec4 clearColor) override { m_ClearColor = clearColor; }
        void Draw   (Scene* scene) override;

		void CreateFullscreenQuad();

        void SetUniformValues(const OpenGLShader* shader, Scene* scene);

    private:
        void InitShaders();

        OpenGLShadowmap* m_Shadowmap{};

        glm::vec4 m_ClearColor = {};
        std::shared_ptr<OpenGLShader> m_ShaderProgram = nullptr;
        std::unique_ptr<OpenGLShader> m_DepthShader = nullptr;

        UniformDirectionalLight m_UniformDirectionalLight{};

        size_t m_PointLightCount{};
        std::array<UniformPointLight, MAX_POINT_LIGHTS> m_UniformPointLights{};

        size_t m_SpotLightCount{};
        std::array<UniformSpotLight, MAX_SPOT_LIGHTS> m_UniformSpotLights{};

        GLuint m_QuadVAO{};
        GLuint m_QuadVBO{};

        int m_Width{};
        int m_Height{};
    };

} 
