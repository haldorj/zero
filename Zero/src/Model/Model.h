#pragma once

#include <glm/glm.hpp>
#include <Renderer/OpenGL/OpenGLShader.h>
#include <vulkan/vulkan.h>
#include <Renderer/Vulkan/vk_types.h>
#include <Renderer/Vulkan/vk_descriptors.h>
#include <map>
#include "Vertex.h"
#include <assimp/scene.h>

namespace Zero {

	struct BoneInfo
	{
		/*id is index in finalBoneMatrices*/
		int ID;

		/*offset matrix transforms vertex from model space to bone space*/
		glm::mat4 Offset;

	};

	class Model
	{
	public:
		virtual ~Model() = default;
		// OpenGL
		virtual void Draw(OpenGLShader& shader, glm::mat4& matrix) {};

		// Vulkan
		virtual void Draw(VkCommandBuffer& cmd, DescriptorWriter& descriptorWriter, VkPipelineLayout& pipelineLayout, VkExtent2D drawExtent, VkSampler& sampler, GPUDrawPushConstants& pushConstants) {}
		virtual void DestroyModel() const {}

		auto& GetBoneInfoMap() { return m_BoneInfoMap; }
		int& GetBoneCount() { return m_BoneCounter; }

	protected:
		void SetVertexBoneDataToDefault(Vertex& vertex);
		void SetVertexBoneData(Vertex& vertex, int boneID, float weight);
		void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, const aiMesh* mesh, const aiScene* scene);

		std::map<std::string, BoneInfo> m_BoneInfoMap; //
		int m_BoneCounter = 0;
	};

}
