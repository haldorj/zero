#pragma once

#include <glad/glad.h>
#include <vector>
#include <Mesh/Vertex.h>

class VBO
{
public:
    VBO() = default;
    VBO(std::vector<Vertex>& vertices);

    void Bind();
    void Unbind();
    void Delete() ;
    ~VBO() = default;

private:
    // Reference ID of the Vertex Buffer Object
    GLuint ID;
};
