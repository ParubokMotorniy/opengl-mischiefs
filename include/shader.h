#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr)
    {
        const std::string &vShaderCode = readShaderSource(vertexPath);
        const std::string &fShaderCode = readShaderSource(fragmentPath);

        const char *vPtr = vShaderCode.c_str();
        const char *fPtr = fShaderCode.c_str();

        unsigned int vertex, fragment;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vPtr, NULL);
        compileShader(vertex);

        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fPtr, NULL);
        compileShader(fragment);

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);

        if (geometryPath != nullptr)
        {
            const std::string &gShaderCode = readShaderSource(geometryPath);

            const char *gPtr = gShaderCode.c_str();

            unsigned int geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gPtr, NULL);
            compileShader(geometry);
            glAttachShader(ID, geometry);

            linkProgram(ID);

            glDeleteShader(geometry);
        }
        else
        {
            linkProgram(ID);
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    ~Shader()
    {
        glDeleteProgram(ID);
    }

    void use() const { glUseProgram(ID); }
    
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setMatrix4(const std::string &name, const glm::mat4 &mat)
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
    void setVec3(const std::string &name, const glm::vec3 &vec)
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), vec.x, vec.y, vec.z);
    }

    operator int()
    {
        return ID;
    }

private:
    void compileShader(uint shaderId)
    {
        glCompileShader(shaderId);

        int success;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shaderId, sizeof(infoLog), NULL, infoLog);
            std::cerr << "Shader compilation failed. Details: " << infoLog;
        }
    }

    void linkProgram(uint programId)
    {
        glLinkProgram(programId);

        int success;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
            std::cerr << "Program linking failed. Details: " << infoLog;
        }
    }

    std::string readShaderSource(const char *shaderSource)
    {
        std::string shaderCode;
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            shaderFile.open(shaderSource);

            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();

            shaderFile.close();

            shaderCode = shaderStream.str();
        }
        catch (std::ifstream::failure &e)
        {
            std::cout << "SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        return shaderCode;
    }

private:
    // the program ID
    unsigned int ID;
    char infoLog[512];
};
