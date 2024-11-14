#pragma once

#include <glad/glad.h>
#include "VBO.h"

class VAO final : public IObjectInterface
{
public:
    // Constructor that generates a VAO ID
    VAO();

    // Links a VBO to the VAO using a certain layout
    void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
    // Binds the VAO
    void Bind() override;
    // Unbinds the VAO
    void Unbind() override;
    // Deletes the VAO
    void Delete() override;

    ~VAO() override = default;

private:
    // ID reference for the Vertex Array Object
    GLuint ID;
};
