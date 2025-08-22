#include "Animator.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace Zero {

    Animator::Animator()
    {
        m_CurrentTime = 0.0;
        m_DeltaTime = 0.0f;

        const int maxBones = 100;

        m_FinalBoneMatrices.reserve(maxBones);

        for (int i = 0; i < maxBones; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    Animator::Animator(Animation* animation)
    {
        m_CurrentTime = 0.0;
        m_DeltaTime = 0.0f;
        m_CurrentAnimation = animation;

        const int maxBones = 100;

        m_FinalBoneMatrices.reserve(maxBones);

        for (int i = 0; i < maxBones; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    void Animator::UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * m_DeltaTime;
            if (m_Loop)
            {
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            }
            else
            {
                if (m_CurrentTime >= m_CurrentAnimation->GetDuration())
                    m_CurrentTime = m_CurrentAnimation->GetDuration() - 0.1;
            }
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void Animator::PlayAnimation(Animation* animation)
    {
        m_CurrentAnimation = animation;
        m_CurrentTime = 0.0f;
    }

    void Animator::PlayAnimation(int32_t index, bool loop)
    {
		m_Loop = loop;
		m_CurrentAnimationIndex = index;
        PlayAnimation(&m_Animations[index]);
    }

    void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        const std::string& nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        if (Bone* bone = m_CurrentAnimation->FindBone(nodeName))
        {
            bone->Update(m_CurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        const auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        auto it = boneInfoMap.find(nodeName);
        if (it != boneInfoMap.end())
        {
            const BoneInfo& info = it->second;
            m_FinalBoneMatrices[info.ID] = globalTransformation * info.Offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }

    std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
    {
        return m_FinalBoneMatrices;
    }

    void Animator::LoadAnimations(const std::string& animationPath, Model* model)
    {
        if (!model)
        {
            std::cerr << "Animator::LoadAnimations: Model is null!" << std::endl;
            return;
        };

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath.c_str(), aiProcess_Triangulate);
        assert(scene && scene->mRootNode);

		m_Animations.clear();
		m_Animations.reserve(scene->mNumAnimations);

        for (int i = 0; i < scene->mNumAnimations; i++)
        {
            Animation anim(animationPath, model, i);
            m_Animations.emplace_back(anim);
        }
    }
}