#pragma once

#include <glm/glm.hpp>

constexpr int MAX_BONE_INFLUENCE = 4;

namespace Zero {

    struct Vertex 
    {
        glm::vec3 Position;
        float UvX;
        glm::vec3 Normal;
        float UvY;
        glm::vec4 Color;
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;

        //bone indexes which will influence this vertex
        int m_BoneIDs[MAX_BONE_INFLUENCE];
        //weights from each bone
        float m_Weights[MAX_BONE_INFLUENCE];
    };

}
