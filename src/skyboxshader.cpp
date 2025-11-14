#include "skyboxshader.h"
#include "meshmanager.h"
#include "texturemanager3d.h"

SkyboxShader::SkyboxShader(MeshIdentifier skyboxMesh, TextureIdentifier3D skyboxTexture)
    : _skyboxMesh(skyboxMesh), _skyboxTexture(skyboxTexture)
{
}
void SkyboxShader::runShader()
{
    if (_skyboxMesh != InvalidIdentifier && _skyboxTexture != InvalidIdentifier)
    {
        use();

        MeshManager::instance()->allocateMesh(_skyboxMesh);
        CubemapManager::instance()->allocateTexture(_skyboxTexture);
        
        MeshManager::instance()->bindMesh(_skyboxMesh);
        const int bindPoint = CubemapManager::instance()->bindTexture(_skyboxTexture);

        if (bindPoint != -1)
        {
            const Mesh *skyMesh = MeshManager::instance()->getMesh(_skyboxMesh);

            const glm::mat4 skyboxModel = glm::scale(glm::identity<glm::mat4>(),
                                                      glm::vec3(-1.0f, -1.0f, -1.0f));
            setInt("skyboxSampler", bindPoint);
            setMatrix4("model", skyboxModel);

            glDrawElements(GL_TRIANGLES, skyMesh->numIndices(), GL_UNSIGNED_INT, 0);
            CubemapManager::instance()->unbindTexture(_skyboxTexture);
        }

        MeshManager::instance()->unbindMesh();
    }
}
void SkyboxShader::compileAndAttachNecessaryShaders(uint32_t id)
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

void SkyboxShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
