#pragma once

#include <glm/glm.hpp>

constexpr int MAX_BONE_INFLUENCE = 4;

namespace Zero {

    struct Vertex 
    {
        glm::vec3 Position{};
        float UvX{};
        glm::vec3 Normal{};
        float UvY{};
        glm::vec4 Color{};

        glm::vec3 Tangent{};
        glm::vec3 Bitangent{};

        //bone indexes which will influence this vertex
        glm::vec<4, int> m_BoneIDs{};
        //weights from each bone
        glm::vec4 m_Weights{};
    };

}
