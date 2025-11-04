#pragma once

#include "material.h"
#include "singleton.h"
#include "texture3d.h"
#include "types.h"

#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>

class TextureManager3D : public SystemSingleton<TextureManager3D> // crtp
{
public:
    friend class SystemSingleton; // so that the singleton can access the private constructor
    using NamedTexture = NamedComponent<Texture3D>;

    TextureIdentifier3D registerTexture(const std::array<const char *, 6> &cubemapSources, const std::string &texName);
    std::pair<std::string, TextureIdentifier3D> registerTexture(const std::array<const char *, 6> &cubemapSources);

    void unregisterTexture(TextureIdentifier3D id);
    TextureIdentifier3D textureRegistered(const std::string &texName) const;

    void allocateTexture(TextureIdentifier3D id);
    void deallocateTexture(TextureIdentifier3D id);

    int bindTexture(TextureIdentifier3D id);
    void unbindTexture(TextureIdentifier3D id);
    void unbindAllTextures();

    void cleanUpGracefully();

    Texture3D *getTexture(TextureIdentifier3D tId);

private:
    TextureManager3D();

    // returns -1 if is not bound and the binding index otherwise
    int isTextureBound(const Texture3D &texture);

private:
    TextureIdentifier3D _identifiers = 0; // TODO: add some defragmentation logic
    std::unordered_map<TextureIdentifier3D, NamedTexture> _textures;

    constexpr static uint32_t MAX_TEXTURES = 16;
    uint32_t _boundTextures[MAX_TEXTURES];
    int _numBoundTextures = 0;
};
