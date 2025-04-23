#include "OpenGLModel.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Model/Vertex.h>

namespace Zero
{
    OpenGLModel::OpenGLModel(const char* path)
    {
        LoadModel(path);
    }

    void OpenGLModel::Draw(OpenGLShader& shader, glm::mat4& matrix)
    {
        for (auto& mesh : m_Meshes)
            mesh.Draw(shader, matrix);
    }

    void OpenGLModel::LoadModel(const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            printf("Model failed to load: %s, %s", path.c_str(), importer.GetErrorString());
            return;
        }
        m_Directory = path.substr(0, path.find_last_of('/'));

        ProcessNode(scene->mRootNode, scene);
    }

    void OpenGLModel::ProcessNode(const aiNode* node, const aiScene* scene)
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

    OpenGLMesh OpenGLModel::ProcessMesh(const aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<OpenGLTexture> textures;

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
            std::vector<OpenGLTexture> diffuseMaps = LoadMaterialTextures(material,
                                                                          aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<OpenGLTexture> specularMaps = LoadMaterialTextures(material,
                                                                           aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        return {vertices, indices, textures};
    }

    std::vector<OpenGLTexture> OpenGLModel::LoadMaterialTextures(const aiMaterial* mat, const aiTextureType type,
                                                                 const std::string& typeName)
    {
        std::vector<OpenGLTexture> textures;
        for (size_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString path;
            mat->GetTexture(type, static_cast<int>(i), &path);
            bool skip = false;
            for (auto& loadedTexture : m_LoadedTextures)
            {
                const size_t idx = std::string(loadedTexture.GetFilePath()).rfind('/');

                if (std::string texName = loadedTexture.GetFilePath().substr(idx + 1); texName == path.C_Str())
                {
                    textures.emplace_back(loadedTexture);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                const size_t idx = std::string(path.C_Str()).rfind('\\');

                std::string tex = std::string(path.data).substr(idx + 1);
                const std::string pathStr = m_Directory + '/' + tex;

                OpenGLTexture texture = OpenGLTexture(pathStr, typeName, static_cast<int>(i), GL_UNSIGNED_BYTE);

                textures.emplace_back(texture);
                m_LoadedTextures.emplace_back(texture);
            }
        }
        return textures;
    }
}
