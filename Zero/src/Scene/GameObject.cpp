#include "GameObject.h"
#include "Core/core.h"

void Zero::GameObject::Update(float dt)
{
	if (!EnablePhysics) return;

	m_Dynamics.Force = m_Dynamics.Mass * GRAVITY;
	m_Dynamics.Velocity += (m_Dynamics.Force / m_Dynamics.Mass) * dt;
	m_Transform.Location += m_Dynamics.Velocity * dt;

	m_Dynamics.Force = glm::vec3(0.0f);

	if (m_Transform.Location.y < -2.0f)
	{
		EnablePhysics = false;
	}
}
