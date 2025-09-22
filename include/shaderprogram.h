#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>

class ShaderProgram
{
public:
    ShaderProgram(const char *vertexPath, const char *fragmentPath);

    void initializeShaderProgram();

    ~ShaderProgram();

    void use() const;

    void setBool(const std::string &name, bool value) const;

    void setInt(const std::string &name, int value) const;

    void setFloat(const std::string &name, float value) const;

    void setMatrix4(const std::string &name, const glm::mat4 &mat);

    void setVec3(const std::string &name, const glm::vec3 &vec);

    void setVec4(const std::string &name, const glm::vec4 &vec);

    operator int()
    {
        return _id;
    }

protected:
    virtual void compileAndAttachNecessaryShaders(uint32_t id);

    virtual void deleteShaders();
    void compileShader(uint32_t shaderId);

    void linkProgram(uint32_t programId);

    std::string readShaderSource(const char *shaderSource);

protected:
    char _infoLog[512];

private:
    unsigned int _id;
    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;
};
