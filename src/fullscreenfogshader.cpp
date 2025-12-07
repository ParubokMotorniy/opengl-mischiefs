#include "fullscreenfogshader.h"
#include "framebuffermanager.h"
#include "meshmanager.h"
#include "texturemanager.h"
#include "volumetricfogcomputepass.h"

#include "glad/glad.h"

FullscreenFogShader::FullscreenFogShader(const VolumetricFogPass *fogPass) : _fogPass(fogPass) {}

void FullscreenFogShader::runShader()
{
    use();

    // TODO: consider using a different blending setup
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    _fogPass->syncTextureAccess(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    const auto colorBinding = TextureManager::instance()->bindTexture(_fogPass->colorTextureId());
    const auto positionBinding = TextureManager::instance()->bindTexture(
        _fogPass->positionTextureId());

    assert(colorBinding != -1 && positionBinding != -1);

    const auto &[viewportOffsetX, viewportOffsetY, viewportSizeX,
                 viewportSizeY] = FrameBufferManager::instance()->getViewportDims();

    setInt("texColor", colorBinding);
    setInt("texPosition", positionBinding);
    setInt("viewportOffsetX", viewportOffsetX);
    setInt("viewportOffsetY", viewportOffsetY);

    MeshManager::instance()->bindMesh(MeshManager::instance()->getDummyMesh());
    glDrawArrays(GL_TRIANGLES, 0, 3);
    MeshManager::instance()->unbindMesh();
}

void FullscreenFogShader::compileAndAttachNecessaryShaders(uint32_t id)
{
    if (_vertexShaderId == 0)
    {
        const std::string &vShaderCode = readShaderSource(_vertexPath);

        const char *vPtr = vShaderCode.c_str();

        _vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(_vertexShaderId, 1, &vPtr, 0);
        compileShader(_vertexShaderId);
    }

    glAttachShader(id, _vertexShaderId);

    if (_fragmentShaderId == 0)
    {
        const std::string &fShaderCode = readShaderSource(_fragmentPath);

        const char *fPtr = fShaderCode.c_str();

        _fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(_fragmentShaderId, 1, &fPtr, 0);
        compileShader(_fragmentShaderId);
    }

    glAttachShader(id, _fragmentShaderId);
}

void FullscreenFogShader::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
