#pragma once

#include <glad/glad.h>
#include "VBO.h"


namespace Zero {

    class VAO final
    {
    public:
        // Constructor that generates a VAO ID
        VAO();

        // Links a VBO to the VAO using a certain layout
        void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
        void LinkAttribINT(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
        // Binds the VAO
        void Bind();
        // Unbinds the VAO
        void Unbind();
        // Deletes the VAO
        void Delete();

        ~VAO() = default;

    private:
        // ID reference for the Vertex Array Object
        GLuint ID;
    };

}
