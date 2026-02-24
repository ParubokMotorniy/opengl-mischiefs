#pragma once

#include "computeshader.h"
#include "framepass.h"
#include "types.h"

class VolumetricFogPass : public FramePass, public ShaderProgram
{
public:
    VolumetricFogPass();
    void runPass() override;
    void syncTextureAccess(uint32_t syncBits) const;
    TextureIdentifier colorTextureId() const;
    TextureIdentifier positionTextureId() const;
    bool volumeIsOutOfSight() const;

    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    struct RenderTargetPair
    {
        TextureIdentifier colorTexture = InvalidIdentifier;
        TextureIdentifier positionTexture = InvalidIdentifier;
    };

    const RenderTargetPair &getCurrentReadTarget() const;
    const RenderTargetPair &getCurrentWriteTarget() const;

private:
    bool _currentPair = false;
    RenderTargetPair _readPair;
    RenderTargetPair _writePair;

    TextureIdentifier _fogTexture = InvalidIdentifier;
    int _numMipLeves = -1;

    bool _volumeIsOutOfSight = false;

    ComputeShader _fogSphereShader{ ENGINE_COMPUTE_SHADERS "/fog_sphere.cs" };

    const char *_vertexPath = ENGINE_SHADERS "/fog_renderer.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/fog_renderer.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
