#include "shaderprogram.h"

#include "glad/glad.h"

ShaderProgram::ShaderProgram(const char *vertexPath, const char *fragmentPath) : _vertexPath(vertexPath), _fragmentPath(fragmentPath)
{
}

void ShaderProgram::initializeShaderProgram()
{
    _id = glCreateProgram();

    const std::string &vShaderCode = readShaderSource(_vertexPath);
    const std::string &fShaderCode = readShaderSource(_fragmentPath);

    const char *vPtr = vShaderCode.c_str();
    const char *fPtr = fShaderCode.c_str();

    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vPtr, NULL);
    compileShader(vertex);

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fPtr, NULL);
    compileShader(fragment);

    glAttachShader(_id, vertex);
    glAttachShader(_id, fragment);
    compileAndAttachNecessaryShaders(_id);

    linkProgram(_id);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    deleteShaders();
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(_id);
}

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

void ShaderProgram::compileAndAttachNecessaryShaders(uint32_t id) {}

void ShaderProgram::deleteShaders() {}

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