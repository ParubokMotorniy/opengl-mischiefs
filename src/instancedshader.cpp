#include "instancedshader.h"
#include "instancer.h"
#include "materialmanager.h"

template <typename MaterialStruct>
InstancedShader<MaterialStruct>::InstancedShader(const char *vertexPath, const char *fragmentPath)
    : _vertexPath(vertexPath), _fragmentPath(fragmentPath)
{
}

template <typename MaterialStruct>
InstancedShader<MaterialStruct>::~InstancedShader()
{
    glDeleteBuffers(1, &_texturesSSBO);
    glDeleteBuffers(_instancedBufferIds.size(), _instancedBufferIds.data());
}

template <typename MaterialStruct>
void InstancedShader<MaterialStruct>::updateInstancedBuffer(
    const std::unordered_set<GameObjectIdentifier> &objsToUpdate)
{
    if (_instancedBufferIds.empty())
        return;

    const std::vector<GameObjectIdentifier> shaderObjects(_orderedShaderObjects.cbegin(),
                                                          _orderedShaderObjects.cend());

    std::vector<std::pair<GameObjectIdentifier, uint32_t>> objsToReinstance;
    size_t meshCounter = 0;
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
                Instancer::instance()->updateInstancedData(_instancedBufferIds[meshCounter++],
                                                           getDataGenerators(), objsToReinstance);
            }
            objsToReinstance.clear();
            meshStart = meshEnd;
        }

        if (meshEnd == shaderObjects.cend())
            break;

        if (objsToUpdate.contains(*meshEnd))
        {
            const size_t objIdx = meshEnd - meshStart;
            objsToReinstance.emplace_back(*meshEnd, objIdx);
        }
        ++meshEnd;
    }
}

template <typename MaterialStruct>
void InstancedShader<MaterialStruct>::runShader()
{
    use();

    for (auto &[meshId, count] : _instancedMeshes)
    {
        const Mesh &mesh = *MeshManager::instance()->getMesh(meshId);

        MeshManager::instance()->bindMeshInstanced(meshId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _texturesSSBO);

        glDrawElementsInstanced(GL_TRIANGLES, mesh.indicesSize(), GL_UNSIGNED_INT, 0, count);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

        MeshManager::instance()->unbindMesh();
    }
}

template <typename MaterialStruct>
void InstancedShader<MaterialStruct>::compileAndAttachNecessaryShaders(uint32_t id)
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

template <typename MaterialStruct>
void InstancedShader<MaterialStruct>::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}

template <typename MaterialStruct>
void InstancedShader<MaterialStruct>::runTextureMapping()
{
    glDeleteBuffers(1, &_texturesSSBO);
    _objectsTextureMappings
        = MaterialManager<MaterialStruct, getComponentTypeForStruct<MaterialStruct>()>::instance()
              ->bindTextures(std::vector<GameObjectIdentifier>(_orderedShaderObjects.cbegin(),
                                                               _orderedShaderObjects.cend()),
                             _textureHandlesbindingPoint, _texturesSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
}

template <typename MaterialStruct>
std::vector<InstancedDataGenerator> InstancedShader<MaterialStruct>::getDataGenerators()
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
    const InstancedDataGenerator modelMatrixCol1 = InstancedDataGenerator{
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

    const InstancedDataGenerator modelMatrixCol2 = InstancedDataGenerator{
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

    const InstancedDataGenerator modelMatrixCol3 = InstancedDataGenerator{
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

    const auto numTexturesInMaterial = getNumTexturesInMaterial<MaterialStruct>();

    // makes the texture indices instanced
    const InstancedDataGenerator standardMaterialIndices = InstancedDataGenerator{
        sizeof(int) * (int)std::ceil((float)numTexturesInMaterial / 4.0f)
            * 4, // to preserve the 16-byte alignment
        numTexturesInMaterial,
        7,
        GL_INT,
        false,
        [this, numTexturesInMaterial](void *destination, GameObjectIdentifier gId) {
            const std::array<int, numTexturesInMaterial> &objectTextureIndices
                = _objectsTextureMappings.at(gId);

            std::memcpy(destination, objectTextureIndices.data(),
                        sizeof(int) * numTexturesInMaterial);
        }
    };

    return { modelMatrixCol0, modelMatrixCol1, modelMatrixCol2, modelMatrixCol3,
             standardMaterialIndices };
}

template <typename MaterialStruct>
void InstancedShader<MaterialStruct>::runInstancing()
{
    const std::vector<GameObjectIdentifier> shaderObjects
        = std::vector<GameObjectIdentifier>(_orderedShaderObjects.cbegin(),
                                            _orderedShaderObjects.cend());

    _instancedMeshes.clear();
    _instancedBufferIds.clear();

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
                MeshManager::instance()->enableMeshInstancing(sharedMesh);
                const Mesh &mesh = *MeshManager::instance()->getMesh(sharedMesh);

                const GLuint vertexBufferid = Instancer::instance()
                                                  ->instanceData(std::span(meshStart, meshEnd),
                                                                 getDataGenerators(),
                                                                 mesh.instancedArrayId());

                _instancedMeshes.emplace(sharedMesh, meshEnd - meshStart);
                _instancedBufferIds.emplace_back(vertexBufferid);
            }
            meshStart = meshEnd;
        }

        if (meshEnd == shaderObjects.cend())
            break;

        ++meshEnd;
    }
}
