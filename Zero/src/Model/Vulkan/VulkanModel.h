#pragma once
#include <vector>
#include <string>
#include <Model/Vulkan/VulkanMesh.h>
#include <assimp/scene.h>
#include <Renderer/Vulkan/VulkanTexture.h>
#include <Renderer/Vulkan/vk_types.h>
#include <Model/Model.h>

namespace Zero {

    class VulkanModel : public Model
    {
    public:
        VulkanModel() = default;
        VulkanModel(const char* path);

        void Draw(
            VkCommandBuffer& cmd, 
            VkPipelineLayout& pipelineLayout, 
            VkExtent2D drawExtent, 
            VkSampler& sampler, 
            GPUDrawPushConstants& pushConstants) override;

        void DestroyModel() const override;

        std::vector<VulkanTexture> GetLoadedTextures() const { return m_LoadedTextures; }

    private:
        void LoadModel(const std::string& path);
        void ProcessNode(const aiNode* node, const aiScene* scene);
        VulkanMesh ProcessMesh(const aiMesh* mesh, const aiScene* scene);
        std::vector<VulkanTexture> LoadMaterialTextures(const aiMaterial* mat, aiTextureType type,
                                                        const std::string& typeName);

        // model data
        std::vector<class VulkanMesh> m_Meshes;
        std::string m_Directory;

        std::vector<VulkanTexture> m_LoadedTextures;

        const aiScene* m_Scene;


    };

}
