#include "standardshader.h"

void StandardShader::compileAndAttachNecessaryShaders(uint32_t id)
{

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
}

void StandardShader::deleteShaders()
{
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void StandardShader::updateUniforms()
{
}

void StandardShader::makeDrawCalls()
{
        const InstancedDataGenerator modelMatrixCol0 = InstancedDataGenerator{sizeof(glm::vec4), 4, 3, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                          {
                                                                              const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrix();

                                                                              std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (0 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                          }};
    InstancedDataGenerator modelMatrixCol1 = InstancedDataGenerator{sizeof(glm::vec4), 4, 4, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                    {
                                                                        const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrix();

                                                                        std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (1 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                    }};

    InstancedDataGenerator modelMatrixCol2 = InstancedDataGenerator{sizeof(glm::vec4), 4, 5, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                    {
                                                                        const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrix();

                                                                        std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (2 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                    }};

    InstancedDataGenerator modelMatrixCol3 = InstancedDataGenerator{sizeof(glm::vec4), 4, 6, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                    {
                                                                        const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrix();

                                                                        std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (3 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                    }};
}
