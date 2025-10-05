#pragma once

#include "shaderprogram.h"

class BasicShader : public ShaderProgram
{
public:
    BasicShader(MeshIdentifier basicMesh, TextureIdentifier basicTexture, size_t objCountX,
                size_t objCountY);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    MeshIdentifier _basicMesh = InvalidIdentifier;
    TextureIdentifier _basicTexture = InvalidIdentifier;

    const char *_vertexPath = ENGINE_SHADERS "/basic_texture.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/basic_texture.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
    size_t _objCountX = 0;
    size_t _objCountY = 0;
};

