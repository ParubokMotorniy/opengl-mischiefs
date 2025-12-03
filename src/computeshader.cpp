#include "computeshader.h"

#include "glad/glad.h"

ComputeShader::ComputeShader(const char *shaderSource) : _shaderSrc(shaderSource) {}

ComputeShader *ComputeShader::setGlobalDispatchDimenisons(const glm::uvec3 &newDimensions)
{
    _globalDispatchDimensions = newDimensions;
    return this;
}

void ComputeShader::runShader()
{
    use();
    glDispatchCompute(_globalDispatchDimensions.x, _globalDispatchDimensions.y,
                      _globalDispatchDimensions.z);
}

void ComputeShader::synchronizeGpuAccess(uint32_t syncBits)
{
    glMemoryBarrier(syncBits);
}

void ComputeShader::compileAndAttachNecessaryShaders(uint32_t id)
{
    if (_computeShaderId == 0)
    {
        const std::string &vShaderCode = readShaderSource(_shaderSrc.c_str());

        const char *vPtr = vShaderCode.c_str();

        _computeShaderId = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(_computeShaderId, 1, &vPtr, NULL);
        compileShader(_computeShaderId);
    }

    glAttachShader(id, _computeShaderId);
}

void ComputeShader::deleteShaders() { glDeleteShader(_computeShaderId); }
