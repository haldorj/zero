#pragma once

#include <glad/glad.h>

class EBO final
{
public:
    EBO() = default;
    // Constructor that generates a Elements Buffer Object and links it to indices
    EBO(GLuint* indices, GLsizeiptr size);

    // Binds the EBO
    void Bind();
    // Unbinds the EBO
    void Unbind();
    // Deletes the EBO
    void Delete();

private:
    // ID reference of Elements Buffer Object
    GLuint ID;
};
