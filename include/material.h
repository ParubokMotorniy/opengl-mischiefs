#pragma once

#include "types.h"

#include <array>

struct BasicMaterial
{
    TextureIdentifier diffTextureName = InvalidIdentifier;
    TextureIdentifier specTextureName = InvalidIdentifier;
    TextureIdentifier emissionTextureName = InvalidIdentifier;

    std::array<TextureIdentifier, 3> textures() const
    {
        return { diffTextureName, specTextureName, emissionTextureName };
    }
};

struct PbrMaterial
{
    TextureIdentifier albedoIdentifier = InvalidIdentifier;
    TextureIdentifier normalIdentifier = InvalidIdentifier;
    TextureIdentifier roughnessIdentifier = InvalidIdentifier;
    TextureIdentifier metallicIdentifier = InvalidIdentifier;
    TextureIdentifier aoIdentifier = InvalidIdentifier;

    std::array<TextureIdentifier, 5> textures() const
    {
        return { albedoIdentifier, normalIdentifier, roughnessIdentifier, metallicIdentifier,
                 aoIdentifier };
    }
};
