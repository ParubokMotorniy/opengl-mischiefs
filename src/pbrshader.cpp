#include "pbrshader.h"

#include "glad/glad.h"

#include "materialmanager.h"
#include "modelloader.h"
#include "types.h"

namespace
{
GameObjectIdentifier gameboyModelId = InvalidIdentifier;
MeshIdentifier gameboyMeshId = 25;
MaterialIdentifier pbrMaterialId = 1;
} // namespace

PbrShader::PbrShader(const char *vertexPath, const char *fragmentPath)
    : _vertexPath(vertexPath), _fragmentPath(fragmentPath)
{
    gameboyModelId = ModelLoader::instance()->loadModel(ENGINE_MODELS "/gameboy/gameboy.obj", false,
                                                        true);
    // gameboyMeshId = MeshManager::instance()->meshRegistered("Gameboy");
    // pbrMaterialId = ObjectManager::instance()
    //                     ->getObject(gameboyModelId)
    //                     .getIdentifierForComponent(ComponentType::PBR_MATERIAL);
}

void PbrShader::runShader()
{
    use();
    const auto pbrMaterial = MaterialManager<PbrMaterial, ComponentType::PBR_MATERIAL>::instance()
                                 ->getMaterial(pbrMaterialId);
    const auto testMesh = MeshManager::instance()->getMesh(gameboyMeshId);
    MeshManager::instance()->allocateMesh(gameboyMeshId);
    MeshManager::instance()->bindMesh(gameboyMeshId);

    TextureManager::instance()->allocateTexture(pbrMaterial.albedoIdentifier);
    TextureManager::instance()->allocateTexture(pbrMaterial.roughnessIdentifier);
    TextureManager::instance()->allocateTexture(pbrMaterial.metallicIdentifier);
    TextureManager::instance()->allocateTexture(pbrMaterial.normalIdentifier);
    TextureManager::instance()->allocateTexture(pbrMaterial.aoIdentifier);

    setMatrix4("model", glm::scale(glm::identity<glm::mat4>(), glm::vec3(10.0f, 10.0f, 10.0f)));
    setInt("albedoMap", TextureManager::instance()->bindTexture(pbrMaterial.albedoIdentifier));
    setInt("roughnessMap",
           TextureManager::instance()->bindTexture(pbrMaterial.roughnessIdentifier));
    setInt("metallicMap", TextureManager::instance()->bindTexture(pbrMaterial.metallicIdentifier));
    setInt("normalMap", TextureManager::instance()->bindTexture(pbrMaterial.normalIdentifier));
    setInt("aoMap", TextureManager::instance()->bindTexture(pbrMaterial.aoIdentifier));

    glDrawElements(GL_TRIANGLES, testMesh->numIndices(), GL_UNSIGNED_INT, 0);

    MeshManager::instance()->unbindMesh();
}

void PbrShader::compileAndAttachNecessaryShaders(uint32_t id)
{
    if (_vertexShaderId == 0)
    {
        const std::string &vShaderCode = readShaderSource(_vertexPath.c_str());

        const char *vPtr = vShaderCode.c_str();

        _vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(_vertexShaderId, 1, &vPtr, 0);
        compileShader(_vertexShaderId);
    }

    glAttachShader(id, _vertexShaderId);

    if (_fragmentShaderId == 0)
    {
        const std::string &fShaderCode = readShaderSource(_fragmentPath.c_str());

        const char *fPtr = fShaderCode.c_str();

        _fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(_fragmentShaderId, 1, &fPtr, 0);
        compileShader(_fragmentShaderId);
    }

    glAttachShader(id, _fragmentShaderId);
}

void PbrShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
