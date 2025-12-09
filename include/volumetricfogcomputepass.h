#pragma once

#include "computeshader.h"
#include "framepass.h"
#include "types.h"

class VolumetricFogPass : public FramePass
{
public:
    VolumetricFogPass();
    void runPass() override;
    void syncTextureAccess(uint32_t syncBits) const;
    TextureIdentifier colorTextureId() const;
    TextureIdentifier positionTextureId() const;

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

    ComputeShader _fogSphereShader{ ENGINE_COMPUTE_SHADERS "/fog_sphere.cs" };
};
