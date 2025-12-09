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
    TextureIdentifier _colorTexture = InvalidIdentifier;
    TextureIdentifier _positionTexture = InvalidIdentifier;
    TextureIdentifier _fogTexture = InvalidIdentifier;

    ComputeShader _fogSphereShader{ ENGINE_COMPUTE_SHADERS "/fog_sphere.cs" };
};
