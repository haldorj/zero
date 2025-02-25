#include "GameObject.h"

#include "Core/Application.h"
#include "Core/core.h"

void Zero::GameObject::Destroy()
{
    m_Collider = nullptr;
    m_Model->DestroyModel();
}
