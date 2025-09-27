#pragma once

#include "texture.h"
#include "material.h"
#include "types.h"
#include "singleton.h"

#include <unordered_map>
#include <string>
#include <cstdint>
#include <tuple>

class TextureManager : public SystemSingleton<TextureManager>
{
public:
    friend class SystemSingleton; // so that the singleton can access the private constructor

    void registerTexture(const char *textureSource, const std::string &texName);
    std::string registerTexture(const char *textureSource);
    void unregisterTexture(const std::string &texName);
    bool textureRegistered(const TextureIdentifier &texName) const;

    void allocateTexture(const std::string &texName);
    void deallocateTexture(const std::string &texName);

    int bindTexture(const std::string &texName);
    void unbindTexture(int textureId);
    void unbindAllTextures();

    std::tuple<int, int, int> bindMaterial(const Material &mat);
    void allocateMaterial(const Material &mat);

    void cleanUpGracefully();

private:
    TextureManager();

    // returns -1 if is not bound and the binding index otherwise
    int isTextureBound(const Texture2D &texture);

private:
    constexpr static uint32_t MAX_TEXTURES = 16;
    std::unordered_map<TextureIdentifier, Texture2D> _textures;

    uint32_t _boundTextures[MAX_TEXTURES];
    int _numBoundTextures = 0;
};
