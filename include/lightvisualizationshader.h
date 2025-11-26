#pragma once

#include "shaderprogram.h"

class LightVisualizationShader : public ShaderProgram
{
public:
    explicit LightVisualizationShader(MeshIdentifier lightMesh);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    MeshIdentifier _lightMesh = InvalidIdentifier;

    const char *_vertexPath = ENGINE_SHADERS "/light_source.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/light_source.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
