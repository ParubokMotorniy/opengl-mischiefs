#include "worldplaneshader.h"

#include "meshmanager.h"
#include "texturemanager.h"

WorldPlaneShader::WorldPlaneShader(MeshIdentifier planeMesh, TextureIdentifier planeTexture)
    : _planeMesh(planeMesh), _planeTexture(planeTexture)
{
}

void WorldPlaneShader::runShader()
{
    if (_planeMesh != InvalidIdentifier && _planeTexture != InvalidIdentifier)
    {
        MeshManager::instance()->allocateMesh(_planeMesh);
        MeshManager::instance()->bindMesh(_planeMesh);

        TextureManager::instance()->allocateTexture(_planeTexture);
        const int bindPoint = TextureManager::instance()->bindTexture(_planeTexture);

        if (bindPoint != -1)
        {
            const Mesh *planeMesh = MeshManager::instance()->getMesh(_planeMesh);

            setInt("planeTexture", bindPoint);
            glm::mat4 squashedCubeMatrix = glm::scale(glm::identity<glm::mat4>(),
                                                      glm::vec3(500.0f, 0.05f, 500.0f));
            squashedCubeMatrix = glm::translate(squashedCubeMatrix, glm::vec3(0.0f, -50.0f, 0.0f));
            setMatrix4("model", squashedCubeMatrix);
            setFloat("checkerUnitWidth", 5.0f);
            setFloat("checkerUnitHeight", 5.0f);

            if (!_planeEnabled)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glDrawElements(GL_TRIANGLES, planeMesh->numIndices(), GL_UNSIGNED_INT, 0);
            TextureManager::instance()->unbindTexture(_planeTexture);

            if (!_planeEnabled)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        MeshManager::instance()->unbindMesh();
    }
}
void WorldPlaneShader::setPlaneEnabled(bool ifEnabled) noexcept { _planeEnabled = ifEnabled; }

bool WorldPlaneShader::isPlaneEnabled() const noexcept { return _planeEnabled; }

void WorldPlaneShader::compileAndAttachNecessaryShaders(uint32_t id)
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

void WorldPlaneShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
