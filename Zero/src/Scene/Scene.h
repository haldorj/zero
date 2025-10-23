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
		Scene(DirectionalLight* light);

		void Destroy() const;

		void AddPointLight(std::shared_ptr<PointLight> pointLight);
		void AddSpotLight(std::shared_ptr<SpotLight> spotLight);
		void AddGameObject(std::shared_ptr<GameObject> gameObject);
		void DestroyGameObject(const GameObject::IdType objectID);

		void SetSkybox(Skybox* skybox) 
		{ 
			m_Skybox.reset(skybox); 
		}

		std::vector<std::shared_ptr<GameObject>>& GetGameObjects() { return m_GameObjects; }
		std::shared_ptr<DirectionalLight>& GetDirectionalLight() { return m_DirectionalLight; }
		Material& GetMaterial() { return m_Material; }
		std::vector<std::shared_ptr<PointLight>>& GetPointLights() { return m_PointLights; }
		std::vector<std::shared_ptr<SpotLight>>& GetSpotLights() { return m_SpotLights; }
		Skybox* GetSkybox() const { return m_Skybox.get(); }

	private:
		Material m_Material{};
		std::unique_ptr<Skybox> m_Skybox{};

		// TODO: none of these need to be shared_ptrs.
		// They can be created on the stack instead.
		std::vector<std::shared_ptr<GameObject>> m_GameObjects{};
		std::shared_ptr<DirectionalLight> m_DirectionalLight{};
		std::vector<std::shared_ptr<PointLight>> m_PointLights{};
		std::vector<std::shared_ptr<SpotLight>> m_SpotLights{};
	};

}


