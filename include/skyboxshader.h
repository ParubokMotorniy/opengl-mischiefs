#pragma once

#include "shaderprogram.h"

#include "utils.h"

class SkyboxShader : public ShaderProgram
{
public:
    SkyboxShader(MeshIdentifier skyboxMesh, TextureIdentifier3D skyboxTexture);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    MeshIdentifier _skyboxMesh = InvalidIdentifier;
    TextureIdentifier _skyboxTexture = InvalidIdentifier;

    const char *_vertexPath = ENGINE_SHADERS "/skybox.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/skybox.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
