#pragma once

#include <glad/glad.h>
#include <vector>

class EBO final
{
public:
    EBO() = default;
    EBO(std::vector<GLuint>& indices);

    void Bind();
    void Unbind();
    void Delete();

    ~EBO() = default;

private:
    // ID reference of Element Buffer Object
    GLuint ID;
};
