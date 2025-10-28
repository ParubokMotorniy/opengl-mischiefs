#include "shaderprogram.h"

#include "instancer.h"
#include "materialmanager.h"
#include "transformmanager.h"

#include "glad/glad.h"

#include <cstdint>
#include <span>

ShaderProgram::ShaderProgram() {}

void ShaderProgram::initializeShaderProgram()
{
    _id = glCreateProgram();
    compileAndAttachNecessaryShaders(_id);
    linkProgram(_id);
    deleteShaders();
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(_id); }

void ShaderProgram::use() const { glUseProgram(_id); }

void ShaderProgram::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(_id, name.c_str()), (int)value);
}
void ShaderProgram::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
}
void ShaderProgram::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(_id, name.c_str()), value);
}
void ShaderProgram::setMatrix4(const std::string &name, const glm::mat4 &mat)
{
    glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
void ShaderProgram::setVec3(const std::string &name, const glm::vec3 &vec)
{
    glUniform3f(glGetUniformLocation(_id, name.c_str()), vec.x, vec.y, vec.z);
}
void ShaderProgram::setVec4(const std::string &name, const glm::vec4 &vec)
{
    glUniform4f(glGetUniformLocation(_id, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}

void ShaderProgram::addObject(GameObjectIdentifier gId) { _orderedShaderObjects.insert(gId); }

void ShaderProgram::addObjectWithChildren(GameObjectIdentifier gId)
{
    GameObject &obj = ObjectManager::instance()->getObject(gId);
    if (!obj.children().empty())
    {
        std::ranges::for_each(obj.children(),
                              [this](GameObjectIdentifier cGId) { addObjectWithChildren(cGId); });
    }
    addObject(gId);
}

void ShaderProgram::compileShader(uint32_t shaderId)
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

void ShaderProgram::linkProgram(uint32_t programId)
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

std::string ShaderProgram::readShaderSource(const char *shaderSource)
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
        std::cout << "Failed to read the shader file: " << e.what() << std::endl;
    }
    return shaderCode;
}