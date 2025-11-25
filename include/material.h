#pragma once

#include "types.h"

#include <array>
#include <stddef.h>
#include <tuple>
#include <type_traits>
#include <cassert>

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

template <typename MaterialStruct, size_t textureCount = std::tuple_size_v<
                                       decltype(std::declval<MaterialStruct>().textures())>>
constexpr size_t getNumTexturesInMaterial()
{
    return textureCount;
};

template <typename MaterialStruct>
constexpr ComponentType getComponentTypeForStruct()
{
    if constexpr (std::is_same_v<MaterialStruct, BasicMaterial>)
        return ComponentType::BASIC_MATERIAL;
    if constexpr (std::is_same_v<MaterialStruct, PbrMaterial>)
        return ComponentType::PBR_MATERIAL;
    assert(false);
}
