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
        //TODO: add lazy evaluation of the shader
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

        _id = glCreateProgram();
        glAttachShader(_id, vertex);
        glAttachShader(_id, fragment);

        if (geometryPath != nullptr)
        {
            const std::string &gShaderCode = readShaderSource(geometryPath);

            const char *gPtr = gShaderCode.c_str();

            unsigned int geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gPtr, NULL);
            compileShader(geometry);
            glAttachShader(_id, geometry);

            linkProgram(_id);

            glDeleteShader(geometry);
        }
        else
        {
            linkProgram(_id);
        }

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    ~Shader()
    {
        glDeleteProgram(_id);
    }

    void use() const { glUseProgram(_id); }
    
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(_id, name.c_str()), (int)value);
    }
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(_id, name.c_str()), value);
    }
    void setMatrix4(const std::string &name, const glm::mat4 &mat)
    {
        glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
    void setVec3(const std::string &name, const glm::vec3 &vec)
    {
        glUniform3f(glGetUniformLocation(_id, name.c_str()), vec.x, vec.y, vec.z);
    }
    void setVec4(const std::string &name, const glm::vec4 &vec)
    {
        glUniform4f(glGetUniformLocation(_id, name.c_str()), vec.x, vec.y, vec.z, vec.w);
    }

    operator int()
    {
        return _id;
    }

private:
    void compileShader(uint shaderId)
    {
        glCompileShader(shaderId);

        int success;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shaderId, sizeof(_infoLog), NULL, _infoLog);
            std::cerr << "Shader compilation failed. Details: " << _infoLog;
        }
    }

    void linkProgram(uint programId)
    {
        glLinkProgram(programId);

        int success;
        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(programId, sizeof(_infoLog), NULL, _infoLog);
            std::cerr << "Program linking failed. Details: " << _infoLog;
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
    // the program _id
    unsigned int _id;
    char _infoLog[512];
};
