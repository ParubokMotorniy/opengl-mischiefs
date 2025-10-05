#pragma once

#include "material.h"
#include "singleton.h"
#include "texture.h"
#include "types.h"

#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>

// TODO: generalize the manager through templates like material manager does
class TextureManager : public SystemSingleton<TextureManager> // crtp
{
public:
    friend class SystemSingleton; // so that the singleton can access the private constructor
    using NamedTexture = NamedComponent<Texture2D>;

    TextureIdentifier registerTexture(const char *textureSource, const std::string &texName);
    std::pair<std::string, TextureIdentifier> registerTexture(const char *textureSource);

    void unregisterTexture(TextureIdentifier id);
    TextureIdentifier textureRegistered(const std::string &texName) const;

    void allocateTexture(TextureIdentifier id);
    void deallocateTexture(TextureIdentifier id);

    int bindTexture(TextureIdentifier id);
    void unbindTexture(TextureIdentifier id);
    void unbindAllTextures();

    std::tuple<int, int, int> bindMaterial(const BasicMaterial &mat); // TODO: rethink this heresy
    void allocateMaterial(const BasicMaterial &mat);

    void cleanUpGracefully();

    Texture2D *getTexture(TextureIdentifier tId);

private:
    TextureManager();

    // returns -1 if is not bound and the binding index otherwise
    int isTextureBound(const Texture2D &texture);

private:
    TextureIdentifier _identifiers = 0; // TODO: add some defragmentation logic
    std::unordered_map<TextureIdentifier, NamedTexture> _textures;

    constexpr static uint32_t MAX_TEXTURES = 16;
    uint32_t _boundTextures[MAX_TEXTURES];
    int _numBoundTextures = 0;
};
