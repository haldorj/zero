#pragma once

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 Position;
    float UvX;
    glm::vec3 Normal;
    float UvY;
    glm::vec4 Color;
};