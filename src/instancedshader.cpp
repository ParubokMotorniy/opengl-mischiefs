#include "instancedshader.h"
#include "instancer.h"
#include "materialmanager.h"

InstancedShader::InstancedShader(const char *vertexPath, const char *fragmentPath)
    : _vertexPath(vertexPath), _fragmentPath(fragmentPath)
{
}

InstancedShader::~InstancedShader()
{
    glDeleteBuffers(1, &_texturesSSBO);
    glDeleteBuffers(_instancedBufferIds.size(), _instancedBufferIds.data());
}

void InstancedShader::runShader()
{
    use();

    for (auto &[meshId, count] : _instancedMeshes)
    {
        const Mesh &mesh = *MeshManager::instance()->getMesh(meshId);

        MeshManager::instance()->bindMesh(meshId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _texturesSSBO);

        glDrawElementsInstanced(GL_TRIANGLES, mesh.indicesSize(), GL_UNSIGNED_INT, 0, count);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

        MeshManager::instance()->unbindMesh();
    }
}

void InstancedShader::compileAndAttachNecessaryShaders(uint32_t id)
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

void InstancedShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}

void InstancedShader::runTextureMapping()
{
    glDeleteBuffers(1, &_texturesSSBO);
    _objectsTextureMappings
        = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
              ->bindTextures(std::vector<GameObjectIdentifier>(_orderedShaderObjects.cbegin(),
                                                               _orderedShaderObjects.cend()),
                             0, _texturesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
}

void InstancedShader::runInstancing()
{
    // TODO: these generators are quite inefficient -> rework
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

    // makes the texture indices instanced
    InstancedDataGenerator standardMaterialIndices = InstancedDataGenerator{
        sizeof(int) * 4, // 4 to preserve the 16-byte alignment
        3,
        7,
        GL_INT,
        false,
        [this](void *destination, GameObjectIdentifier gId) {
            const std::array<int, 3> &objectTextureIndices = _objectsTextureMappings.at(gId);

            std::memcpy(destination, objectTextureIndices.data(), sizeof(int) * 3);
        }
    };
    const std::vector<GameObjectIdentifier> shaderObjects(_orderedShaderObjects.cbegin(),
                                                          _orderedShaderObjects.cend());

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
            if (sharedMesh != InvalidIdentifier)
            {
                MeshManager::instance()->allocateMesh(sharedMesh);
                const Mesh &mesh = *MeshManager::instance()->getMesh(sharedMesh);

                const GLuint vertexBufferId = Instancer::instance()
                                                  ->instanceData(std::span(meshStart, meshEnd),
                                                                 { modelMatrixCol0, modelMatrixCol1,
                                                                   modelMatrixCol2, modelMatrixCol3,
                                                                   standardMaterialIndices },
                                                                 mesh);

                _instancedMeshes.emplace(sharedMesh, meshEnd - meshStart);
                _instancedBufferIds.emplace_back(vertexBufferId);
            }
            meshStart = meshEnd;
        }

        ++meshEnd;
    }
}
