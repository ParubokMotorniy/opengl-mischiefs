#include "lightvisualizationshader.h"
#include "lightmanager.h"

#include "instancer.h"

LightVisualizationShader::LightVisualizationShader(MeshIdentifier lightMesh) : _lightMesh(lightMesh)
{
}

void LightVisualizationShader::runShader()
{
    const InstancedDataGenerator modelMatrixCol0 = InstancedDataGenerator{
        sizeof(glm::vec4),
        4,
        3,
        GL_FLOAT,
        false,
        [](void *destination, GameObjectIdentifier gId) {
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
        [](void *destination, GameObjectIdentifier gId) {
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
        [](void *destination, GameObjectIdentifier gId) {
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
        [](void *destination, GameObjectIdentifier gId) {
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

    InstancedDataGenerator lightSourceColor = InstancedDataGenerator{
        sizeof(glm::vec4),
        4,
        7,
        GL_FLOAT,
        false,
        [](void *destination, GameObjectIdentifier gId) {
            glm::vec4 finalColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

            auto &gObject = ObjectManager::instance()->getObject(gId);

            if (LightSourceIdentifier lId = gObject.getIdentifierForComponent(
                    ComponentType::LIGHT_POINT);
                lId != InvalidIdentifier)
            {
                finalColor += glm::vec4(LightManager<ComponentType::LIGHT_POINT>::instance()
                                            ->getLight(lId)
                                            ->diffuse,
                                        0.0f);
            }

            if (LightSourceIdentifier lId = gObject.getIdentifierForComponent(
                    ComponentType::LIGHT_SPOT);
                lId != InvalidIdentifier)
            {
                finalColor += glm::vec4(LightManager<ComponentType::LIGHT_SPOT>::instance()
                                            ->getLight(lId)
                                            ->diffuse,
                                        0.0f);
            }

            if (LightSourceIdentifier lId = gObject.getIdentifierForComponent(
                    ComponentType::LIGHT_DIRECTIONAL);
                lId != InvalidIdentifier)
            {
                finalColor += glm::vec4(LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                                            ->getLight(lId)
                                            ->diffuse,
                                        0.0f);
            }

            std::memcpy(destination, (void *)&finalColor, sizeof(glm::vec4));
        }
    };

    if (_lightMesh != InvalidIdentifier)
    {
        use();

        const std::vector<GameObjectIdentifier> shaderObjects(_orderedShaderObjects.cbegin(),
                                                              _orderedShaderObjects.cend());

        const Mesh &mesh = *MeshManager::instance()->getMesh(_lightMesh);
        MeshManager::instance()->allocateMesh(_lightMesh);

        const uint32_t vertexBufferId = Instancer::instance()->instanceData(shaderObjects,
                                                                            { modelMatrixCol0,
                                                                              modelMatrixCol1,
                                                                              modelMatrixCol2,
                                                                              modelMatrixCol3,
                                                                              lightSourceColor },
                                                                            mesh);
        MeshManager::instance()->bindMesh(_lightMesh);

        glDrawElementsInstanced(GL_TRIANGLES, mesh.indicesSize(), GL_UNSIGNED_INT, 0,
                                shaderObjects.size());

        MeshManager::instance()->unbindMesh();
        glDeleteBuffers(1, &vertexBufferId); // presumably, it will anyway be regenerated
    }
}

void LightVisualizationShader::compileAndAttachNecessaryShaders(uint32_t id)
{
    if (_vertexShaderId == 0)
    {
        const std::string &vShaderCode = readShaderSource(_vertexPath);

        const char *vPtr = vShaderCode.c_str();

        _vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(_vertexShaderId, 1, &vPtr, NULL);
        compileShader(_vertexShaderId);
    }

    glAttachShader(id, _vertexShaderId);

    if (_fragmentShaderId == 0)
    {
        const std::string &fShaderCode = readShaderSource(_fragmentPath);

        const char *fPtr = fShaderCode.c_str();

        _fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(_fragmentShaderId, 1, &fPtr, NULL);
        compileShader(_fragmentShaderId);
    }

    glAttachShader(id, _fragmentShaderId);
}

void LightVisualizationShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
