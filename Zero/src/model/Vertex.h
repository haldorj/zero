#pragma once

#include <glm/glm.hpp>

class Vertex
{
public:
	Vertex() = default;
	Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord)
		: m_Position(position), m_Normal(normal), m_TexCoord(texCoord) {}

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetNormal() const { return m_Normal; }
	const glm::vec2& GetTexCoord() const { return m_TexCoord; }

private:
    glm::vec3 m_Position;
    glm::vec3 m_Normal;
    glm::vec2 m_TexCoord;
};

