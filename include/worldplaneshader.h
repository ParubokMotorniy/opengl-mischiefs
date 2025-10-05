#pragma once

#include "shaderprogram.h"
#include "types.h"

class WorldPlaneShader : public ShaderProgram
{
public:
    WorldPlaneShader( MeshIdentifier planeMesh,
                     TextureIdentifier planeTexture);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    MeshIdentifier _planeMesh = InvalidIdentifier;
    TextureIdentifier _planeTexture = InvalidIdentifier;

    const char *_vertexPath = ENGINE_SHADERS "/world_plane.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/world_plane.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
