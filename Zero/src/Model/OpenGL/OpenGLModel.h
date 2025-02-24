#pragma once
#include <vector>
#include <string>
#include <Model/OpenGL/OpenGLMesh.h>

#include <assimp/scene.h>
#include <Renderer/OpenGL/OpenGLTexture.h>
#include <Model/Model.h>

namespace Zero {

    class OpenGLModel : public Model
    {
    public:
        OpenGLModel() = default;
        OpenGLModel(const char* path);
        void Draw(OpenGLShader& shader, glm::mat4& matrix) override;

    private:
        void LoadModel(const std::string& path);
        void ProcessNode(const aiNode* node, const aiScene* scene);
        OpenGLMesh ProcessMesh(const aiMesh* mesh, const aiScene* scene);
        std::vector<OpenGLTexture> LoadMaterialTextures(const aiMaterial* mat, aiTextureType type,
                                                        const std::string& typeName);

        std::vector<OpenGLMesh> m_Meshes;
        std::string m_Directory{};

        std::vector<OpenGLTexture> m_LoadedTextures;
        VAO m_Vao{};

        const aiScene* scene;
    };

}