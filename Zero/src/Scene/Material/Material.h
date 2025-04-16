#pragma once

namespace Zero {

	class Material
	{
	public:
		Material() = default;
		Material(float specularIntensity, float shine);

		float GetSpecularIntensity() const { return m_SpecularIntensity; }
		float GetShininess() const { return m_Shininess; }
		
	private:
		float m_SpecularIntensity{};
		float m_Shininess{};
	};

}
