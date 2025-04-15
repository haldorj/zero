#include "Scene.h"

namespace Zero {



	Scene::Scene()
	{
		m_GameObjects.reserve(1000);
		m_DirectionalLight = std::make_shared<Light>();
	}

	Scene::Scene(Light* light)
	{
		m_GameObjects.reserve(1000);
		m_DirectionalLight.reset(light);
	}

	void Scene::Destroy()
	{
		for (auto gameObj : m_GameObjects)
		{
			if (!gameObj)
				continue;
			if (gameObj->GetModel())
				gameObj->GetModel()->DestroyModel();
		}
	}

	void Scene::AddGameObject(std::shared_ptr<GameObject> gameObject)
	{
		m_GameObjects.emplace_back(gameObject);
	}

	void Scene::DestroyGameObject(const GameObject::IdType objectID)
	{
		for (auto it = m_GameObjects.begin(); it != m_GameObjects.end(); ++it)
		{
			if ((*it)->GetID() == objectID)
			{
				(*it)->Destroy();
				m_GameObjects.erase(it);

				break;
			}
		}
	}

}