#pragma once

#include <vector>
#include <memory>
#include "GameObject.h"
#include "Material/Material.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Renderer/OpenGL/OpenGLUtils.h"

namespace Zero {

	class Scene
	{
	public:
		Scene();
		Scene(DirectionalLight* light);

		void Destroy();

		void AddPointLight(std::shared_ptr<PointLight> pointLight);
		void AddGameObject(std::shared_ptr<GameObject> gameObject);
		void DestroyGameObject(const GameObject::IdType objectID);

		std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return m_GameObjects; }
		std::shared_ptr<DirectionalLight>& GetDirectionalLight() { return m_DirectionalLight; }
		std::shared_ptr<Material>& GetMaterial() { return m_Material; }
		std::vector<std::shared_ptr<PointLight>>& GetPointLights() { return m_PointLights; }

	private:
		std::vector<std::shared_ptr<GameObject>> m_GameObjects{};
		std::shared_ptr<DirectionalLight> m_DirectionalLight{};
		std::vector<std::shared_ptr<PointLight>> m_PointLights{};
		std::shared_ptr<Material> m_Material{};
	};

}


