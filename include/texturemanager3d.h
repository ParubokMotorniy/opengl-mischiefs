#pragma once

#include "material.h"
#include "singleton.h"
#include "texture3d.h"
#include "types.h"

#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

class CubemapManager : public SystemSingleton<CubemapManager> // crtp
{
public:
    friend class SystemSingleton; // so that the singleton can access the private constructor
    using NamedTexture = NamedComponent<Cubemap>;

    TextureIdentifier3D registerTexture(const std::array<const char *, 6> &cubemapSources,
                                        const std::string &texName);
    TextureIdentifier3D registerTexture(const std::array<std::string, 6> &cubemapSources,
                                        const std::string &texName);
    std::pair<std::string, TextureIdentifier3D> registerTexture(
        const std::array<const char *, 6> &cubemapSources);

    void unregisterTexture(TextureIdentifier3D id);
    TextureIdentifier3D textureRegistered(const std::string &texName) const;

    void allocateTexture(TextureIdentifier3D id);
    void deallocateTexture(TextureIdentifier3D id);

    int bindTexture(TextureIdentifier3D id);
    void unbindTexture(TextureIdentifier3D id);
    void unbindAllTextures();

    void cleanUpGracefully();

    Cubemap *getTexture(TextureIdentifier3D tId);

private:
    CubemapManager();

    // returns -1 if is not bound and the binding index otherwise
    int isTextureBound(const Cubemap &texture) const;

private:
    TextureIdentifier3D _identifiers = 0; // TODO: add some defragmentation logic
    std::unordered_map<TextureIdentifier3D, NamedTexture> _textures;

    constexpr static uint32_t MAX_TEXTURES = 16;
    uint32_t _boundTextures[MAX_TEXTURES];
    int _numBoundTextures = 0;
};
