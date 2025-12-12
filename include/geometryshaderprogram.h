#pragma once

#include "shaderprogram.h"
#include "types.h"

class GeometryShaderProgram : public ShaderProgram
{
public:
    GeometryShaderProgram(
        const char *vertexPath, const char *fragmentPath, const char *geometryPath); // opengl may not allow to run a shader with no vertex buffer bound
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;
    const char *_geometryPath = nullptr;

    uint32_t _vertexShaderId = 0;
    uint32_t _geometryShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
