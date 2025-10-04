#pragma once

#include "types.h"

#include <array>

struct BasicMaterial
{
    TextureIdentifier diffTextureName = 0;
    TextureIdentifier specTextureName = 0;
    TextureIdentifier emissionTextureName = 0;

    std::array<TextureIdentifier, 3> textures() const
    {
        return { diffTextureName, specTextureName, emissionTextureName };
    }
};
