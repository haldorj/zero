#include"VAO.h"

namespace Zero {
    // Constructor that generates a VAO ID
    VAO::VAO()
    {
        glGenVertexArrays(1, &ID);
        if (ID == 0) {
            // Handle error
            printf("Error generating VAO\n");
        }
    }

    // Links a VBO to the VAO using a certain layout
    void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
    {
        VBO.Bind();
        glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
        glEnableVertexAttribArray(layout);
        VBO.Unbind();
    }

    void VAO::LinkAttribINT(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
    {
        VBO.Bind();
        glVertexAttribIPointer(layout, numComponents, type, stride, offset);
        glEnableVertexAttribArray(layout);
        VBO.Unbind();
    }

    // Binds the VAO
    void VAO::Bind()
    {
        glBindVertexArray(ID);
    }

    // Unbinds the VAO
    void VAO::Unbind()
    {
        glBindVertexArray(0);
    }

    // Deletes the VAO
    void VAO::Delete()
    {
        glDeleteVertexArrays(1, &ID);
    }
}
