#include "basicshader.h"

#include "meshmanager.h"
#include "texturemanager.h"

BasicShader::BasicShader(MeshIdentifier basicMesh, TextureIdentifier basicTexture, size_t objCountX,
                         size_t objCountY)
    : _basicMesh(basicMesh),
      _basicTexture(basicTexture),
      _objCountX(objCountX),
      _objCountY(objCountY)
{
}

void BasicShader::runShader()
{
    if (_basicMesh != InvalidIdentifier && _basicTexture != InvalidIdentifier)
    {
        MeshManager::instance()->allocateMesh(_basicMesh);
        MeshManager::instance()->bindMesh(_basicMesh);

        TextureManager::instance()->allocateTexture(_basicTexture);
        const int bindPoint = TextureManager::instance()->bindTexture(_basicTexture);

        if (bindPoint != -1)
        {
            const Mesh *planeMesh = MeshManager::instance()->getMesh(_basicMesh);

            setInt("basicTexture", bindPoint);

            for (int f = 0; f < _objCountX; ++f)
            {
                for (int x = 0; x < _objCountY; ++x)
                {
                    glm::mat4 model = glm::scale(glm::identity<glm::mat4>(),
                                            glm::vec3(5.0f, 5.0f, 5.0f));
                    model = glm::translate(model, glm::vec3((f - (int)_objCountX / 2) * 7, 0.0f,
                                                            (x - (int)_objCountY / 2) * 7));
                    setMatrix4("model", model);

                    glDrawElements(GL_TRIANGLES, planeMesh->numIndices(), GL_UNSIGNED_INT, 0);
                }
            }
        }

        MeshManager::instance()->unbindMesh();
        TextureManager::instance()->unbindTexture(_basicTexture);
    }
}

void BasicShader::compileAndAttachNecessaryShaders(uint32_t id)
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

void BasicShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
