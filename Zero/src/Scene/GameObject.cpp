#include "GameObject.h"

#include "Core/Application.h"
#include "Core/core.h"


namespace Zero {

    GameObject GameObject::Create()
    {
        static IdType currentID = 0;
        return {currentID++};
    }

    void GameObject::Destroy() const
    {
        if (Application::Get().GetRendererType() == RendererAPI::OpenGL)
        {
            return;
        }
        
        if (m_Model)
        {
            m_Model->DestroyModel();
        }
    }
    
}

