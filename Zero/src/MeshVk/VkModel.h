#pragma once
#include <vector>
#include <string>
#include <MeshVk/VkMesh.h>

#include <assimp/scene.h>
#include <Renderer/Vulkan/VulkanTexture.h>
#include <Renderer/Vulkan/vk_types.h>

namespace Zero {

    class VkModel
    {
    public:
        VkModel() = default;
        VkModel(const char* path);
        void Draw(VkCommandBuffer& cmd, VkPipelineLayout& pipelineLayout, VkExtent2D drawExtent, VkSampler& sampler, GPUDrawPushConstants& pushConstants);

        std::vector<VulkanTexture> GetLoadedTextures() const { return LoadedTextures; }

    private:
        // model data
        std::vector<class VkMesh> meshes;
        std::string directory;

        std::vector<VulkanTexture> LoadedTextures;

        const aiScene* scene;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        VkMesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<VulkanTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };

}
