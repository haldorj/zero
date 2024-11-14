#pragma once

#include <glad/glad.h>

class VBO
{
public:
    // Constructor that generates a Vertex Buffer Object and links it to vertices
    VBO(GLfloat* vertices, GLsizeiptr size);

    void Bind();
    void Unbind();
    void Delete();

private:
    // Reference ID of the Vertex Buffer Object
    GLuint ID;
};
