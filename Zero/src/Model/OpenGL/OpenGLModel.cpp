#include "OpenGLModel.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <algorithm>
#include <Model/Vertex.h>

namespace Zero {

	OpenGLModel::OpenGLModel(const char* path)
	{
		LoadModel(path);
	}

	void OpenGLModel::Draw(OpenGLShader& shader)
	{
		for (auto& mesh : meshes)
			mesh.Draw(shader);
	}

	void OpenGLModel::LoadModel(std::string path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			printf("Model failed to load: %s, %s", path.c_str(), importer.GetErrorString());
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}

	void OpenGLModel::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// process all the node's meshes (if any)
		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scene));
		}
		// then do the same for each of its children
		for (size_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	OpenGLMesh OpenGLModel::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<OpenGLTexture> textures;

		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
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

			vertices.push_back(vertex);
		}

		// process indices
		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// process material
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<OpenGLTexture> diffuseMaps = LoadMaterialTextures(material,
				aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<OpenGLTexture> specularMaps = LoadMaterialTextures(material,
				aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return OpenGLMesh(vertices, indices, textures);
	}

    std::vector<OpenGLTexture> OpenGLModel::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<OpenGLTexture> textures;
        for (size_t i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString path;
            mat->GetTexture(type, i, &path);
            bool skip = false;
            for (unsigned int j = 0; j < LoadedTextures.size(); j++)
            {
				size_t idx = std::string(LoadedTextures[j].GetFilePath()).rfind('/');
				std::string texName = LoadedTextures[j].GetFilePath().substr(idx + 1);

                if (texName == path.C_Str())
                {
                    textures.push_back(LoadedTextures[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
				size_t idx = std::string(path.C_Str()).rfind('\\');

				std::string tex = std::string(path.data).substr(idx + 1);
				std::string pathStr = directory + '/' + tex;

                OpenGLTexture texture = OpenGLTexture(pathStr.c_str(), typeName.c_str(), i, GL_UNSIGNED_BYTE);

                textures.push_back(texture);
                LoadedTextures.push_back(texture);
            }
        }
        return textures;
    }

}

