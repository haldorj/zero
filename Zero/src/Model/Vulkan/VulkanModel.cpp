#include "VulkanModel.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <algorithm>

#include <Renderer/VulkanRenderer.h>
#include <Application.h>
#include <Renderer/Vulkan/VulkanBuffer.h>

namespace Zero
{
    VulkanModel::VulkanModel(const char* path)
    {
        LoadModel(path);
    }

    void VulkanModel::Draw(VkCommandBuffer& cmd, VkPipelineLayout& pipelineLayout,
                           VkSampler& sampler, GPUDrawPushConstants& pushConstants, std::shared_ptr<Animator> animator)
    {
        const auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        //// STORAGE BUFFER //////////////////////////////////////////////////////////////////////////////////////////
        //allocate a new uniform buffer for the scene data
        AllocatedBuffer gpuObjectDataBuffer = VulkanBufferManager::CreateBuffer(renderer->GetAllocator(),
            sizeof(GPUObjectData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        //add it to the deletion queue of this frame so it gets deleted once its been used
        renderer->GetCurrentFrame().DeletionQueue.PushFunction([=, this]() {
            VulkanBufferManager::DestroyBuffer(renderer->GetAllocator(), gpuObjectDataBuffer);
            });

        //write the buffer
        GPUObjectData* objectUniformData = (GPUObjectData*)gpuObjectDataBuffer.Info.pMappedData;
        *objectUniformData = m_GPUObjectData;

        if (animator)
        {
            objectUniformData->Animated = true;

            const std::vector transforms = animator->GetFinalBoneMatrices();
            for (int i = 0; i < transforms.size(); ++i)
            {
                if (i >= 100) break;

                objectUniformData->BoneMatrices[i] = transforms[i];
            }
        }
        else
        {
            objectUniformData->Animated = false;
        }

        DescriptorWriter descriptorWriter{};

        descriptorWriter.WriteBuffer(1, gpuObjectDataBuffer.Buffer, sizeof(GPUObjectData), 0,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

        ///////////////////////////////////////////////////////////////////////////////////////////////////////

        for (unsigned int i = 0; i < m_Meshes.size(); i++)
            m_Meshes[i].Draw(cmd, pipelineLayout, sampler, pushConstants, descriptorWriter);
    }

    void VulkanModel::DestroyModel() const
    {
        for (auto mesh : m_Meshes)
        {
            mesh.DestroyMesh();
        }
        for (auto texture : m_LoadedTextures)
        {
            texture.DestroyImage();
        }
    }

    void VulkanModel::LoadModel(const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            printf("Model failed to load: %s", (path + " " + importer.GetErrorString()).c_str());
            return;
        }
        m_Directory = path.substr(0, path.find_last_of('/'));

        ProcessNode(scene->mRootNode, scene);
    }

    void VulkanModel::ProcessNode(const aiNode* node, const aiScene* scene)
    {
        // process all the node's meshes (if any)
        for (size_t i = 0; i < node->mNumMeshes; i++)
        {
            const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_Meshes.emplace_back(ProcessMesh(mesh, scene));
        }
        // then do the same for each of its children
        for (size_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    VulkanMesh VulkanModel::ProcessMesh(const aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<VulkanTexture> textures;

        for (size_t i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            // process vertex positions, normals and texture coordinates
            vertex.Position =
                glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            if (mesh->HasNormals())
            {
                vertex.Normal =
                    glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            else
            {
                vertex.Normal =
                    glm::vec3(0.0f, 0.0f, 0.0f);
            }

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                vertex.UvX = mesh->mTextureCoords[0][i].x;
                vertex.UvY = mesh->mTextureCoords[0][i].y;
            }
            else
            {
                vertex.UvX = 0;
                vertex.UvY = 0;
            }
            
            vertices.emplace_back(vertex);
        }

        // process indices
        for (size_t i = 0; i < mesh->mNumFaces; i++)
        {
            const aiFace face = mesh->mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
                indices.emplace_back(face.mIndices[j]);
        }

        // process material
        if (mesh->mMaterialIndex >= 0)
        {
            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<VulkanTexture> diffuseMaps = LoadMaterialTextures(material,
                                                                          aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<VulkanTexture> specularMaps = LoadMaterialTextures(material,
                                                                           aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        return VulkanMesh(vertices, indices, textures);
    }

    std::vector<VulkanTexture> VulkanModel::LoadMaterialTextures(const aiMaterial* mat, const aiTextureType type,
                                                                 const std::string& typeName)
    {
        auto renderer = dynamic_cast<VulkanRenderer*>(Application::Get().GetRenderer());

        std::vector<VulkanTexture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString path;
            mat->GetTexture(type, i, &path);
            bool skip = false;
            for (unsigned int j = 0; j < m_LoadedTextures.size(); j++)
            {
                const size_t idx = std::string(m_LoadedTextures[j].GetFilePath().c_str()).rfind('/');

                if (std::string texName = std::string(m_LoadedTextures[j].GetFilePath().c_str()).substr(idx + 1);
                    texName == path.C_Str())
                {
                    textures.emplace_back(m_LoadedTextures[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                const size_t idx = std::string(path.C_Str()).rfind('\\');

                std::string tex = std::string(path.data).substr(idx + 1);
                const std::string pathStr = m_Directory + '/' + tex;

                VulkanTexture texture = VulkanTexture(pathStr, typeName, true);

                textures.emplace_back(texture);
                m_LoadedTextures.emplace_back(texture);
            }
        }

        return textures;
    }
}
