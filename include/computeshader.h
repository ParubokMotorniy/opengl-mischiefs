#pragma once

#include "glm/glm.hpp"
#include "shaderprogram.h"

class ComputeShader final : public ShaderProgram
{
public:
    ComputeShader(const char *shaderSource);
    const ComputeShader *setGlobalDispatchDimenisons(const glm::uvec3 &newDimensions);
    void runShader() override;
    void synchronizeGpuAccess(uint32_t syncBits);

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    uint32_t _computeShaderId = 0;
    std::string _shaderSrc;
    glm::uvec3 _globalDispatchDimensions;
};
