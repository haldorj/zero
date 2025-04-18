#include "Scene.h"

namespace Zero {

	Scene::Scene()
	{
		m_GameObjects.reserve(100);
		m_PointLights.reserve(MAX_POINTLIGHTS);
		m_DirectionalLight = std::make_shared<DirectionalLight>();
	}

	Scene::Scene(DirectionalLight* light)
	{
		m_GameObjects.reserve(100);
		m_PointLights.reserve(MAX_POINTLIGHTS);
		m_DirectionalLight.reset(light);

		// Dull
		m_Material = std::make_shared<Material>(0.25f, 2.0f);

		// Shiny
		// m_Material = std::make_shared<Material>(1.0f, 32.0f);
	}

	void Scene::Destroy()
	{
		for (auto& gameObj : m_GameObjects)
		{
			if (!gameObj)
			{
				continue;
			}

			if (gameObj->GetModel())
			{
				gameObj->GetModel()->DestroyModel();
			}
		}
	}

	void Scene::AddPointLight(std::shared_ptr<PointLight> pointLight)
	{
		if (m_PointLights.size() >= MAX_POINTLIGHTS)
		{
			return;
		}

		m_PointLights.emplace_back(pointLight);
	}

	void Scene::AddGameObject(std::shared_ptr<GameObject> gameObject)
	{
		m_GameObjects.emplace_back(gameObject);
	}

	void Scene::DestroyGameObject(const GameObject::IdType objectID)
	{
		for (auto it = m_GameObjects.begin(); it != m_GameObjects.end(); ++it)
		{
			if ((*it)->GetID() != objectID)
			{
				continue;
			}
				
			(*it)->Destroy();
			m_GameObjects.erase(it);

			break;
		}
	}

}