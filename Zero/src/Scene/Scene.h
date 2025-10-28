#pragma once

#include <vector>
#include <memory>
#include "GameObject.h"
#include "Material/Material.h"
#include "Light/DirectionalLight.h"
#include "Light/PointLight.h"
#include "Renderer/OpenGL/OpenGLUtils.h"
#include "Light/SpotLight.h"
#include "Skybox/Skybox.h"

namespace Zero {

	class Scene
	{
	public:
		Scene();
		Scene(DirectionalLight light);

		void Destroy() const;

		void AddPointLight(PointLight pointLight);
		void AddSpotLight(SpotLight spotLight);
		void AddGameObject(std::shared_ptr<GameObject> gameObject);
		void DestroyGameObject(const GameObject::IdType objectID);

		void SetSkybox(Skybox* skybox);

		std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return m_GameObjects; }
		DirectionalLight& GetDirectionalLight() { return m_DirectionalLight; }
		Material& GetMaterial() { return m_Material; }
		std::vector<PointLight>& GetPointLights() { return m_PointLights; }
		std::vector<SpotLight>& GetSpotLights() { return m_SpotLights; }
		Skybox* GetSkybox() const { return m_Skybox.get(); }

	private:
		Material m_Material{};
		DirectionalLight m_DirectionalLight{};
		std::unique_ptr<Skybox> m_Skybox{};

		std::vector<std::shared_ptr<GameObject>> m_GameObjects{};
		std::vector<PointLight> m_PointLights{};
		std::vector<SpotLight> m_SpotLights{};
	};

}


