#pragma once
#include <vector>
#include <string>
#include <Mesh/Mesh.h>

#include <assimp/scene.h>
#include <Renderer/OpenGL/OpenGLTexture.h>


namespace Zero {
    class Model
    {
    public:
        Model(const char* path);
        void Draw(OpenGLShader& shader);

    private:
        // model data
        std::vector<class Mesh> meshes;
        std::string directory;

        std::vector<OpenGLTexture> textures_loaded;
        VAO VAO;

        const aiScene* scene;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<OpenGLTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };
}