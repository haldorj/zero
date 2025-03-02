#pragma once

#include <array>
#include <glm/glm.hpp>
#include <algorithm>
#include "renderer/Renderer.h"

namespace Zero
{
    class Simplex
    {
    public:
        Simplex() = default;

        Simplex& operator=(const std::initializer_list<glm::vec3> list)
        {
            for (auto v = list.begin(); v != list.end(); v++)
            {
                m_Points[std::distance(list.begin(), v)] = *v;
            }
            m_Size = static_cast<unsigned int>(list.size());
            return *this;
        }

        void PushFront(const glm::vec3& point)
        {
            m_Points = {point, m_Points[0], m_Points[1], m_Points[2]};
            m_Size = std::min(m_Size + 1, 4u);
        }

        glm::vec3& operator[](const unsigned int index) { return m_Points[index]; }
        unsigned int Size() const { return m_Size; }

        auto begin() { return m_Points.begin(); }
        auto end() { return m_Points.begin() - (4 - m_Size); }

    private:
        std::array<glm::vec3, 4> m_Points{};
        unsigned int m_Size{};
    };

    inline bool SameDirection(const glm::vec3& a, const glm::vec3& b)
    {
        return glm::dot(a, b) > 0;
    }

    bool Line(Simplex& points, glm::vec3& direction);
    bool Triangle(Simplex& points, glm::vec3& direction);
    bool Tetrahedron(Simplex& points, glm::vec3& direction);

    inline bool NextSimplex(Simplex& points, glm::vec3 direction)
    {
        switch (points.Size())
        {
        case 2: return Line(points, direction);
        case 3: return Triangle(points, direction);
        case 4: return Tetrahedron(points, direction);
        default: ;
        }
        return false;
    }
}