#include "OpenGLShader.h"
#include <array>

namespace Zero {

    void OpenGLShader::CompileErrors(const unsigned int shader, const char* type)
    {
        // Stores status of compilation
        GLint hasCompiled;
        std::array<char, 1024> infoLog{};
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
            if (hasCompiled == GL_FALSE)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog.data());
                std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog.data() << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
            if (hasCompiled == GL_FALSE)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog.data());
                std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog.data() << std::endl;
            }
        }
    }

    std::string ReadFile(const char* filename)
    {
        std::ifstream in(filename, std::ios::binary);
        if (in)
        {
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            in.close();
            return (contents);
        }
        std::cout << "Error reading file: " << filename << '\n';
        throw(errno);
    }

    OpenGLShader::OpenGLShader(const char* vertexFile, const char* fragmentFile)
    {
        const std::string vertexCode = ReadFile(vertexFile);
        const std::string fragmentCode = ReadFile(fragmentFile);

        const char* vertexSource = vertexCode.c_str();
        const char* fragmentSource = fragmentCode.c_str();

        // Create Vertex Shader Object and get its reference
        const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        // Attach Vertex Shader source to the Vertex Shader Object
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        // Compile the Vertex Shader into machine code
        glCompileShader(vertexShader);
        CompileErrors(vertexShader, "Vertex");

        // Create Fragment Shader Object and get its reference
        const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        // Attach Fragment Shader source to the Fragment Shader Object
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        // Compile the Fragment Shader into machine code
        glCompileShader(fragmentShader);
        CompileErrors(fragmentShader, "Fragment");

        // Create and cache Shader Program Object
        ID = glCreateProgram();
        // Attach the Vertex and Fragment Shaders to the Shader Program
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        // Wrap-up/Link all the shaders together into the Shader Program
        glLinkProgram(ID);

        CompileErrors(ID, "Program");

        // Delete the now useless Vertex and Fragment Shader objects
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void OpenGLShader::Activate() const
    {
        glUseProgram(ID);
    }

    void OpenGLShader::Delete() const
    {
        glDeleteProgram(ID);
    }

}
