#include "geometryshaderprogram.h"

#include "instancer.h"
#include "transformmanager.h"

GeometryShaderProgram::GeometryShaderProgram(const char *vertexPath, const char *fragmentPath,
                                             const char *geometryPath)
    : _vertexPath(vertexPath), _fragmentPath(fragmentPath), _geometryPath{ geometryPath }
{
}

void GeometryShaderProgram::runShader()
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
                      ->computeModelMatrixNoScale();

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
                      ->computeModelMatrixNoScale();

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
                      ->computeModelMatrixNoScale();

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
                      ->computeModelMatrixNoScale();

            std::memcpy(destination,
                        reinterpret_cast<const int8_t *>(&modelMat) + (3 * sizeof(glm::vec4)),
                        sizeof(glm::vec4));
        }
    };

    use();
    MeshIdentifier dummyMesh = MeshManager::instance()->getDummyMesh();

    const std::vector<GameObjectIdentifier> shaderObjects(_orderedShaderObjects.cbegin(),
                                                          _orderedShaderObjects.cend());

    MeshManager::instance()->enableMeshInstancing(dummyMesh);
    const Mesh &mesh = *MeshManager::instance()->getMesh(dummyMesh);

    const uint32_t vertexBufferId = Instancer::instance()->instanceData(shaderObjects,
                                                                        { modelMatrixCol0,
                                                                          modelMatrixCol1,
                                                                          modelMatrixCol2,
                                                                          modelMatrixCol3 },
                                                                        mesh.instancedArrayId());

    MeshManager::instance()->allocateMesh(dummyMesh);
    MeshManager::instance()->bindMeshInstanced(dummyMesh);

    setFloat("axisLength", 0.25l);
    setFloat("thickness", 0.002l);

    glDrawArraysInstanced(GL_POINTS, 0, 3, shaderObjects.size());

    glDeleteBuffers(1, &vertexBufferId); // presumably, it will anyway be regenerated

    MeshManager::instance()->unbindMesh();
}

void GeometryShaderProgram::compileAndAttachNecessaryShaders(uint32_t id)
{
    if (id == 0)
        return;

    if (_geometryShaderId == 0)
    {
        const std::string &gShaderCode = readShaderSource(_geometryPath);

        const char *gPtr = gShaderCode.c_str();

        _geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(_geometryShaderId, 1, &gPtr, NULL);
        compileShader(_geometryShaderId);
    }

    glAttachShader(id, _geometryShaderId);

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

void GeometryShaderProgram::deleteShaders()
{
    glDeleteShader(_geometryShaderId);
    _geometryShaderId = 0;

    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
