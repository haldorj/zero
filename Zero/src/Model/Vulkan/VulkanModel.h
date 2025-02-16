#pragma once
#include <vector>
#include <string>
#include <Model/Vulkan/VulkanMesh.h>
#include <assimp/scene.h>
#include <Renderer/Vulkan/VulkanTexture.h>
#include <Renderer/Vulkan/vk_types.h>

namespace Zero {

    class VulkanModel
    {
    public:
        VulkanModel() = default;
        VulkanModel(const char* path);
        void Draw(VkCommandBuffer& cmd, VkPipelineLayout& pipelineLayout, VkExtent2D drawExtent, VkSampler& sampler, GPUDrawPushConstants& pushConstants);

        void DestroyModel();

        std::vector<VulkanTexture> GetLoadedTextures() const { return LoadedTextures; }

    private:
        // model data
        std::vector<class VulkanMesh> meshes;
        std::string directory;

        std::vector<VulkanTexture> LoadedTextures;

        const aiScene* scene;

        void LoadModel(std::string path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        VulkanMesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<VulkanTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
    };

}
