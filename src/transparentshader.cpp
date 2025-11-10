#include "transparentshader.h"

#include "glad/glad.h"

#include "materialmanager.h"
#include "objectmanager.h"

#include <ranges>
#include <array>

namespace
{
    const std::array<std::string, 2> textureUniformNames = {"diffuseTextureHandle","specularTextureHandle"};
}

TransparentShader::TransparentShader(const char *vertexPath, const char *fragmentPath)
    : _vertexPath(vertexPath), _fragmentPath(fragmentPath)
{
}

void TransparentShader::runShader()
{
    if (_sortedObjects.size() != _orderedShaderObjects.size())
    {
        _sortedObjects = std::vector<GameObjectIdentifier>{ _orderedShaderObjects.cbegin(),
                                                            _orderedShaderObjects.cend() };

        _minimalPreviousDistance = 0;
    }

    if (_sortedObjects.empty())
        return;

    const auto currentCameraPosition = _currentCamera->position();
    const auto currentDelta = currentCameraPosition - _previousCameraPosition;
    bool objectsNeedResorting
        = glm::dot(currentDelta, currentDelta)
          > _minimalPreviousDistance; // if camera has moved further than the closest object

    if (objectsNeedResorting)
    {
        std::ranges::sort(
            _sortedObjects,
            [currentCameraPosition](Transform *t1, Transform *t2) -> bool {
                const auto delta1 = currentCameraPosition - t1->position();
                const auto delta2 = currentCameraPosition - t2->position();
                return glm::dot(delta1, delta1) > glm::dot(delta2, delta2);
            },
            [](GameObjectIdentifier id) -> Transform * {
                const auto tId = ObjectManager::instance()->getObject(id).getIdentifierForComponent(
                    ComponentType::TRANSFORM);
                Transform *tPtr = TransformManager::instance()->getTransform(tId);
                return tPtr;
            });

        // updates the closest object
        const auto minDelta = currentCameraPosition
                              - TransformManager::instance()
                                    ->getTransform(
                                        ObjectManager::instance()
                                            ->getObject(_sortedObjects[0])
                                            .getIdentifierForComponent(ComponentType::TRANSFORM))
                                    ->position();
        _minimalPreviousDistance = glm::dot(minDelta, minDelta);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    for (const GameObjectIdentifier gId : std::ranges::reverse_view(_sortedObjects))
    {
        const MeshIdentifier sharedMesh = ObjectManager::instance()
                                              ->getObject(gId)
                                              .getIdentifierForComponent(ComponentType::MESH);
        if (sharedMesh != InvalidIdentifier)
        {
            // painfully draw objects one by one with no instancing or batching whatsoever
            // TODO: fix this inefficiency
            const Mesh &mesh = *MeshManager::instance()->getMesh(sharedMesh);
            MeshManager::instance()->allocateMesh(mesh.standardArrayId());
            MeshManager::instance()->bindMesh(mesh.standardArrayId());

            const MaterialIdentifier mId
                = ObjectManager::instance()->getObject(gId).getIdentifierForComponent(
                    ComponentType::BASIC_MATERIAL);

            if (mId == InvalidIdentifier)
                return; // some objects may not have materials on them

            const BasicMaterial &mS
                = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
                      ->getMaterial(mId);

            const auto textureIdentifiers = mS.textures();

            for (int t = 0; t < 2; ++t)
            {
                const TextureIdentifier tId = textureIdentifiers[t];
                TextureManager::instance()->allocateTexture(tId);
#if ENGINE_DISABLE_BINDLESS_TEXTURES
                setUvec2(textureUniformNames[t], 0);
#else
                const auto texHandle = glGetTextureHandleARB(
                    *TextureManager::instance()->getTexture(tId));
                assert(texHandle != 0);
                glMakeTextureHandleResidentARB(texHandle);
                setUvec2(textureUniformNames[t], texHandle);
#endif
            }

            glDrawElements(GL_TRIANGLES, mesh.numIndices(), GL_UNSIGNED_INT, 0);

            MeshManager::instance()->unbindMesh();
        }
    }

    _previousCameraPosition = currentCameraPosition;

    // auto meshStart = _sortedObjects.cbegin();
    // auto meshEnd = _sortedObjects.cbegin();
    // while (meshEnd <= _sortedObjects.cend())
    // {
    //     // submits ranges of objects that share the same mesh
    //     if (meshEnd == _sortedObjects.cend()
    //         || (ObjectManager::instance()
    //                 ->getObject(*meshStart)
    //                 .getIdentifierForComponent(ComponentType::MESH)
    //             != ObjectManager::instance()->getObject(*meshEnd).getIdentifierForComponent(
    //                 ComponentType::MESH)))
    //     {
    //         const MeshIdentifier sharedMesh = ObjectManager::instance()
    //                                               ->getObject(*meshStart)
    //                                               .getIdentifierForComponent(ComponentType::MESH);
    //         if (sharedMesh != InvalidIdentifier)
    //         {
    //             MeshManager::instance()->enableMeshInstancing(sharedMesh);
    //             const Mesh &mesh = *MeshManager::instance()->getMesh(sharedMesh);

    //             const GLuint vertexBufferid = Instancer::instance()
    //                                               ->instanceData(std::span(meshStart, meshEnd),
    //                                                              getDataGenerators(),
    //                                                              mesh.instancedArrayId());

    //             _instancedMeshes.emplace(sharedMesh, meshEnd - meshStart);
    //             _instancedBufferIds.emplace_back(vertexBufferid);
    //         }
    //         meshStart = meshEnd;
    //     }

    //     if (meshEnd == _sortedObjects.cend())
    //         break;

    //     ++meshEnd;
    // }
}

void TransparentShader::setCamera(const Camera *newCamera) { _currentCamera = newCamera; }

void TransparentShader::compileAndAttachNecessaryShaders(uint32_t id)
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

void TransparentShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
