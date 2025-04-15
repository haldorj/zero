#pragma once

#include <vector>
#include <memory>
#include "GameObject.h"
#include "Light/Light.h"

namespace Zero {

	class Scene
	{
	public:
		Scene();
		Scene(Light* light);

		void Destroy();

		void AddGameObject(std::shared_ptr<GameObject> gameObject);
		void DestroyGameObject(const GameObject::IdType objectID);

		std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return m_GameObjects; }
		std::shared_ptr<Light>& GetDirectionalLight() { return m_DirectionalLight; }

	private:
		std::vector<std::shared_ptr<GameObject>> m_GameObjects{};
		std::shared_ptr<Light> m_DirectionalLight{};
	};

}


