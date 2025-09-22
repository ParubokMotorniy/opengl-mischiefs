#pragma once

#include "texture.h"

#include <unordered_map>
#include <string>
#include <cstdint>

class TextureManager
{
public:
    static TextureManager *instance();

    void registerTexture(const std::string &texName, const char *textureSource);
    void allocateTexture(const std::string &texName);
    int bindTexture(const std::string &texName);
    void unbindTexture(int textureId);
    void unbindAllTextures();
    void deallocateTexture(const std::string &texName);
    void unregisterTexture(const std::string &texName);
    void cleanUpGracefully();

private:
    TextureManager();

    // returns -1 if is not bound and the binding index otherwise
    int isTextureBound(const Texture2D &texture);

private:
    static TextureManager *_instance;
    constexpr static uint32_t MAX_TEXTURES = 16;
    std::unordered_map<std::string, Texture2D> _textures;

    uint32_t _boundTextures[MAX_TEXTURES];
    int _numBoundTextures = 0;
};
