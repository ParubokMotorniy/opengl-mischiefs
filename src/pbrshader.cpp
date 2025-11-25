#include "pbrshader.h"

#include "glad/glad.h"

#include "materialmanager.h"
#include "modelloader.h"
#include "types.h"

PbrShader::PbrShader(const char *vertexPath, const char *fragmentPath)
    : InstancedShader<PbrMaterial>(vertexPath, fragmentPath)
{
    _textureHandlesbindingPoint = 1;
}

void PbrShader::runShader()
{
    use();

    for (auto &[meshId, count] : _instancedMeshes)
    {
        MeshManager::instance()->enableMeshInstancing(meshId);
        const Mesh &mesh = *MeshManager::instance()->getMesh(meshId);

        MeshManager::instance()->bindMeshInstanced(meshId);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _texturesSSBO);

        glDrawElementsInstanced(GL_TRIANGLES, mesh.indicesSize(), GL_UNSIGNED_INT, 0, count);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

        MeshManager::instance()->unbindMesh();
    }
}

std::vector<InstancedDataGenerator> PbrShader::getDataGenerators()
{
    // TODO: these generators are quite inefficient -> rework
    const InstancedDataGenerator modelMatrixCol0 = InstancedDataGenerator{
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
                        reinterpret_cast<const int8_t *>(&modelMat) + (0 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };
    const InstancedDataGenerator modelMatrixCol1 = InstancedDataGenerator{
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
                        reinterpret_cast<const int8_t *>(&modelMat) + (1 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };

    const InstancedDataGenerator modelMatrixCol2 = InstancedDataGenerator{
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
                        reinterpret_cast<const int8_t *>(&modelMat) + (2 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };

    const InstancedDataGenerator modelMatrixCol3 = InstancedDataGenerator{
        sizeof(glm::vec4),
        4,
        7,
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

    // makes the PBR texture indices instanced
    const InstancedDataGenerator pbrMaterialIndices1 = InstancedDataGenerator{
        sizeof(int) * 4, // 4 to preserve the 16-byte alignment
        4,
        8,
        GL_INT,
        false,
        [this](void *destination, GameObjectIdentifier gId) {
            const std::array<int, 5> &objectTextureIndices = _objectsTextureMappings.at(gId);

            std::memcpy(destination, objectTextureIndices.data(), sizeof(int) * 4);
        }
    };

    // second component is ivec4 to reserve space for future extensions of PBR
    const InstancedDataGenerator pbrMaterialIndices2 = InstancedDataGenerator{
        sizeof(int) * 4, // 4 to preserve the 16-byte alignment
        4,
        9,
        GL_INT,
        false,
        [this](void *destination, GameObjectIdentifier gId) {
            const std::array<int, 5> &objectTextureIndices = _objectsTextureMappings.at(gId);

            std::memcpy(destination, objectTextureIndices.data() + 4, sizeof(int));
        }
    };

    return { modelMatrixCol0, modelMatrixCol1,     modelMatrixCol2,
             modelMatrixCol3, pbrMaterialIndices1, pbrMaterialIndices2 };
}
