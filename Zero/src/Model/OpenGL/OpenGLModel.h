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
        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        OpenGLMesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<OpenGLTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);

        std::vector<class OpenGLMesh> meshes;
        std::string directory{};

        std::vector<OpenGLTexture> LoadedTextures;
        VAO VAO{};

        const aiScene* scene;
    };

}