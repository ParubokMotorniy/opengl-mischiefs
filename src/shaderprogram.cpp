#include "shaderprogram.h"

#include "instancer.h"
#include "materialmanager.h"
#include "transformmanager.h"

#include "glad/glad.h"

#include <cstdint>
#include <span>

ShaderProgram::ShaderProgram(const char *vertexPath, const char *fragmentPath)
    : _vertexPath(vertexPath), _fragmentPath(fragmentPath)
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

void ShaderProgram::addObject(GameObjectIdentifier gId) { _orderedShaderObjects.push(gId); }

void ShaderProgram::runShader() // TODO: make instancing virtual as well
{
    const InstancedDataGenerator modelMatrixCol0 = InstancedDataGenerator{
        sizeof(glm::vec4),
        4,
        3,
        GL_FLOAT,
        false,
        [](int8_t *destination, GameObjectIdentifier gId) {
            const glm::mat4 modelMat
                = TransformManager::instance()
                      ->getTransform(
                          ObjectManager::instance()->getObject(gId).getIdentifierForComponent(
                              ComponentType::TRANSFORM))
                      ->computeModelMatrix();

            std::memcpy(destination,
                        reinterpret_cast<const int8_t *>(&modelMat) + (0 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };
    InstancedDataGenerator modelMatrixCol1 = InstancedDataGenerator{
        sizeof(glm::vec4),
        4,
        4,
        GL_FLOAT,
        false,
        [](int8_t *destination, GameObjectIdentifier gId) {
            const glm::mat4 modelMat
                = TransformManager::instance()
                      ->getTransform(
                          ObjectManager::instance()->getObject(gId).getIdentifierForComponent(
                              ComponentType::TRANSFORM))
                      ->computeModelMatrix();

            std::memcpy(destination,
                        reinterpret_cast<const int8_t *>(&modelMat) + (1 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };

    InstancedDataGenerator modelMatrixCol2 = InstancedDataGenerator{
        sizeof(glm::vec4),
        4,
        5,
        GL_FLOAT,
        false,
        [](int8_t *destination, GameObjectIdentifier gId) {
            const glm::mat4 modelMat
                = TransformManager::instance()
                      ->getTransform(
                          ObjectManager::instance()->getObject(gId).getIdentifierForComponent(
                              ComponentType::TRANSFORM))
                      ->computeModelMatrix();

            std::memcpy(destination,
                        reinterpret_cast<const int8_t *>(&modelMat) + (2 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };

    InstancedDataGenerator modelMatrixCol3 = InstancedDataGenerator{
        sizeof(glm::vec4),
        4,
        6,
        GL_FLOAT,
        false,
        [](int8_t *destination, GameObjectIdentifier gId) {
            const glm::mat4 modelMat
                = TransformManager::instance()
                      ->getTransform(
                          ObjectManager::instance()->getObject(gId).getIdentifierForComponent(
                              ComponentType::TRANSFORM))
                      ->computeModelMatrix();

            std::memcpy(destination,
                        reinterpret_cast<const int8_t *>(&modelMat) + (3 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };

    // TODO: in the future, bind the textures bindlessly
    use();
    const std::vector<GameObjectIdentifier> &shaderObjects = getContainer(_orderedShaderObjects);

    uint32_t uniformTexturesIdx;
    const uint32_t materialsBinding = 0;
    const auto objectIndices
        = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
              ->bindTextures(shaderObjects, materialsBinding, uniformTexturesIdx);

    // unsigned int lights_index = glGetUniformBlockIndex(_id, "Lights");
    // glUniformBlockBinding(_id, lights_index, materialsBinding);
    // glBindBuffer(GL_UNIFORM_BUFFER, uniformTexturesIdx);

    // makes the texture indices instanced
    InstancedDataGenerator standardMaterialIndices = InstancedDataGenerator{
        sizeof(int) * 3,
        3,
        7,
        GL_INT,
        false,
        [&](int8_t *destination, GameObjectIdentifier gId) {
            const std::array<int, 3> &objectTextureIndices = objectIndices.at(gId);

            std::memcpy(destination, objectTextureIndices.data(), sizeof(int) * 3);
        }
    };

    auto meshStart = shaderObjects.cbegin();
    auto meshEnd = shaderObjects.cbegin();
    while (meshEnd <= shaderObjects.cend())
    {
        // submits ranges of objects that share the same mesh
        if (meshEnd == shaderObjects.cend()
            || (ObjectManager::instance()
                    ->getObject(*meshStart)
                    .getIdentifierForComponent(ComponentType::MESH)
                != ObjectManager::instance()->getObject(*meshEnd).getIdentifierForComponent(
                    ComponentType::MESH)))
        {
            const MeshIdentifier sharedMesh = ObjectManager::instance()
                                                  ->getObject(*meshStart)
                                                  .getIdentifierForComponent(ComponentType::MESH);
            const Mesh &mesh = *MeshManager::instance()->getMesh(sharedMesh);

            const uint32_t vertexBufferId = Instancer::instance()
                                                ->instanceData(std::span(meshStart, meshEnd),
                                                               { modelMatrixCol0, modelMatrixCol1,
                                                                 modelMatrixCol2, modelMatrixCol3,
                                                                 standardMaterialIndices },
                                                               mesh);

            MeshManager::instance()->bindMesh(sharedMesh);

            glDrawElementsInstanced(GL_TRIANGLES, mesh.indicesSize(), GL_UNSIGNED_INT, 0,
                                    meshEnd - meshStart);

            glDeleteBuffers(1, &vertexBufferId); // presumably, it will anyway be regenerated

            MeshManager::instance()->unbindMesh();
            meshStart = meshEnd;
        }

        ++meshEnd;
    }

    glDeleteBuffers(1, &uniformTexturesIdx); // TODO: this is inefficient
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