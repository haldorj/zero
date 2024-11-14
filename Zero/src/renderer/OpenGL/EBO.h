#pragma once

#include <glad/glad.h>

#include "Interfaces\IObjectInterface.h"

class EBO final : public IObjectInterface
{
public:
    EBO() = default;
    // Constructor that generates a Elements Buffer Object and links it to indices
    EBO(GLuint* indices, GLsizeiptr size);

    // Binds the EBO
    void Bind() override;
    // Unbinds the EBO
    void Unbind() override;
    // Deletes the EBO
    void Delete() override;

    ~EBO() override = default;

private:
    // ID reference of Elements Buffer Object
    GLuint ID;
};
