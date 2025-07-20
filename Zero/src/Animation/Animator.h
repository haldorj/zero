#pragma once  
#include <vector>  
#include "Animation.h"  
#include "Model/Model.h"  

namespace Zero {  

class Animator  
{  
public:  
	Animator();  
	Animator(Animation* animation);  
	void UpdateAnimation(float dt);  
	void PlayAnimation(Animation* animation);
	void PlayAnimation(int32_t index, bool loop = true);
	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);  

	std::vector<glm::mat4> GetFinalBoneMatrices();
	int32_t GetCurrentAnimationIndex() const { return m_CurrentAnimationIndex; }

	void LoadAnimations(const std::string& animationPath, Model* model);  

private:  
	std::vector<glm::mat4> m_FinalBoneMatrices{};
	std::vector<Animation> m_Animations{};  
	Animation* m_CurrentAnimation{};
	int32_t m_CurrentAnimationIndex{ 0 };
	float m_CurrentTime{};
	float m_DeltaTime{};
	bool m_Loop{ true };
};  

}
