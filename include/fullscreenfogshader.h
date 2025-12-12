#pragma once

#include "shaderprogram.h"

class VolumetricFogPass;

class FullscreenFogShader : public ShaderProgram
{
public:
    FullscreenFogShader(const VolumetricFogPass *fogPass);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    const VolumetricFogPass *_fogPass = nullptr;

    const char *_vertexPath = ENGINE_SHADERS "/fog_renderer.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/fog_renderer.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
