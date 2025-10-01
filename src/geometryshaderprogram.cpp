#include "geometryshaderprogram.h"

#include "instancer.h"
#include "transformmanager.h"

GeometryShaderProgram::GeometryShaderProgram(const char *vertexPath,
                                             const char *fragmentPath,
                                             const char *geometryPath)
    : ShaderProgram(vertexPath, fragmentPath),
      _geometryPath{geometryPath}
{
}

void GeometryShaderProgram::runShader()
{
    const InstancedDataGenerator modelMatrixCol0 = InstancedDataGenerator{sizeof(glm::vec4), 4, 3, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                          {
                                                                              const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrixNoScale();

                                                                              std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (0 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                          }};
    InstancedDataGenerator modelMatrixCol1 = InstancedDataGenerator{sizeof(glm::vec4), 4, 4, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                    {
                                                                        const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrixNoScale();

                                                                        std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (1 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                    }};

    InstancedDataGenerator modelMatrixCol2 = InstancedDataGenerator{sizeof(glm::vec4), 4, 5, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                    {
                                                                        const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrixNoScale();

                                                                        std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (2 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                    }};

    InstancedDataGenerator modelMatrixCol3 = InstancedDataGenerator{sizeof(glm::vec4), 4, 6, GL_FLOAT, false, [](int8_t *destination, GameObjectIdentifier gId)
                                                                    {
                                                                        const glm::mat4 modelMat = TransformManager::instance()->getTransform(ObjectManager::instance()->getObject(gId).getIdentifierForComponent(ComponentType::TRANSFORM))->computeModelMatrixNoScale();

                                                                        std::memcpy(destination, reinterpret_cast<const int8_t *>(&modelMat) + (3 * sizeof(glm::vec4)), sizeof(glm::vec4));
                                                                    }};

    // TODO: in the future, bind the textures bindlessly
    use();

    const std::vector<GameObjectIdentifier> &shaderObjects = getContainer(_orderedShaderObjects);
    auto meshStart = shaderObjects.cbegin();
    auto meshEnd = shaderObjects.cbegin();
    while (meshEnd <= shaderObjects.cend())
    {
        // submits ranges of objects that share the same mesh
        if (meshEnd == shaderObjects.cend() || (ObjectManager::instance()->getObject(*meshStart).getIdentifierForComponent(ComponentType::MESH) != ObjectManager::instance()->getObject(*meshEnd).getIdentifierForComponent(ComponentType::MESH)))
        {
            const MeshIdentifier sharedMesh = ObjectManager::instance()->getObject(*meshStart).getIdentifierForComponent(ComponentType::MESH);
            const Mesh &mesh = *MeshManager::instance()->getMesh(sharedMesh);

            const uint32_t vertexBufferId = Instancer::instance()->instanceData(std::span(meshStart, meshEnd), {modelMatrixCol0, modelMatrixCol1, modelMatrixCol2, modelMatrixCol3}, mesh);

            MeshManager::instance()->bindMesh(sharedMesh);

            glDrawElementsInstanced(GL_TRIANGLES, mesh.indicesSize(), GL_UNSIGNED_INT, 0, meshEnd - meshStart);
            glDrawArraysInstanced(GL_POINTS, 0, 3, meshEnd - meshStart);

            glDeleteBuffers(1, &vertexBufferId); // presumably, it will anyway be regenerated

            MeshManager::instance()->unbindMesh();
            meshStart = meshEnd;
        }

        ++meshEnd;
    }
}

void GeometryShaderProgram::compileAndAttachNecessaryShaders(uint id)
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
}

void GeometryShaderProgram::deleteShaders()
{
    glDeleteShader(_geometryShaderId);
    _geometryShaderId = 0;
}
