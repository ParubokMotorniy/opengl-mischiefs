#pragma once

#include "shaderprogram.h"
#include "types.h"

class GeometryShaderProgram : public ShaderProgram
{
public:
    GeometryShaderProgram(
        const char *vertexPath, const char *fragmentPath, const char *geometryPath,
        MeshIdentifier
            dummyMesh); // opengl may not allow to run a shader with no vertex buffer bound
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint id) override;
    void deleteShaders() override;

private:
    MeshIdentifier _dummyMesh = InvalidIdentifier;

    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;
    const char *_geometryPath = nullptr;

    uint32_t _vertexShaderId = 0;
    uint32_t _geometryShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
