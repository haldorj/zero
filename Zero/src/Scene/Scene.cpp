#include "Scene.h"

namespace Zero {

	Scene::Scene()
	{
		m_GameObjects.reserve(100);
		m_PointLights.reserve(MAX_POINT_LIGHTS);
		m_SpotLights.reserve(MAX_SPOT_LIGHTS);
		m_DirectionalLight = std::make_shared<DirectionalLight>();
	}

	Scene::Scene(DirectionalLight* light)
	{
		m_GameObjects.reserve(100);
		m_PointLights.reserve(MAX_POINT_LIGHTS);
		m_SpotLights.reserve(MAX_SPOT_LIGHTS);
		m_DirectionalLight.reset(light);

		// Dull
		m_Material = std::make_shared<Material>(0.25f, 4.0f);

		// Shiny
		// m_Material = std::make_shared<Material>(1.0f, 32.0f);
	}

	void Scene::Destroy() const
	{
		m_Skybox->Destroy();
	}

	void Scene::AddPointLight(std::shared_ptr<PointLight> pointLight)
	{
		if (m_PointLights.size() >= MAX_POINT_LIGHTS)
		{
			return;
		}

		m_PointLights.emplace_back(pointLight);
	}

	void Scene::AddSpotLight(std::shared_ptr<SpotLight> spotLight)
	{
		if (m_SpotLights.size() >= MAX_SPOT_LIGHTS)
		{
			return;
		}

		m_SpotLights.emplace_back(spotLight);
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