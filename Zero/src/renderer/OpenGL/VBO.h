#pragma once

#include <glad/glad.h>

#include "Interfaces\IObjectInterface.h"

class VBO : public IObjectInterface
{
public:
    // Constructor that generates a Vertex Buffer Object and links it to vertices
    VBO(GLfloat* vertices, GLsizeiptr size);

    void Bind() override;
    void Unbind() override;
    void Delete() override;
    ~VBO() override = default;

private:
    // Reference ID of the Vertex Buffer Object
    GLuint ID;
};
