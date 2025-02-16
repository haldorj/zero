#pragma once
#include <vector>
#include <string>
#include <MeshVk/VkMesh.h>

#include <assimp/scene.h>
#include <Renderer/Vulkan/vk_types.h>

namespace Zero {
    class VkModel
    {
    public:
        VkModel(const char* path);
        void Draw();

    private:
        // model data
        std::vector<class VkMesh> meshes;
        std::string directory;

        std::vector<AllocatedImage> textures_loaded;

        const aiScene* scene;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        VkMesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<AllocatedImage> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };
}