#pragma once

#include "glm/glm.hpp"
#include "shaderprogram.h"

class ComputeShader : public ShaderProgram
{
public:
    ComputeShader(const char *shaderSource);
    ComputeShader *setGlobalDispatchDimenisons(const glm::uvec3 &newDimensions);
    void runShader() override;
    void synchronizeGpuAccess(uint32_t syncBits) const;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) final override;
    void deleteShaders() final override;

private:
    uint32_t _computeShaderId = 0;
    std::string _shaderSrc;

protected:
    glm::uvec3 _globalDispatchDimensions;
};
