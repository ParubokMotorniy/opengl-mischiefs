#pragma once

#include "shaderprogram.h"

class GeometryShaderProgram : public ShaderProgram
{
public:
    GeometryShaderProgram(const char *vertexPath,
                          const char *fragmentPath,
                          const char *geometryPath = nullptr);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint id) override;
    void deleteShaders() override;

private:
    const char *_geometryPath = nullptr;
    uint _geometryShaderId = 0;
};
