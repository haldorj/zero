#include "GameObject.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

void Zero::GameObject::Update() const
{
	if (m_Model)
	{
		m_Model->SetMatrix(glm::translate(glm::mat4(1.0f), m_Transform.Location) *
			glm::rotate(glm::mat4(1.0f), m_Transform.Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), m_Transform.Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(glm::mat4(1.0f), m_Transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
			glm::scale(glm::mat4(1.0f), m_Transform.Scale));
	}
}
