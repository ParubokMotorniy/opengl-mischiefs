#include "texturemanager.h"
#include <cassert>
#include <cstring>
#include <glad/glad.h>

#include "randomnamer.h"

TextureManager *TextureManager::instance()
{
    static TextureManager instance;
    return &instance;
}

std::string TextureManager::registerTexture(const char *textureSource)
{
    const auto rName = RandomNamer::instance()->getRandomName(10);
    registerTexture(textureSource, rName);
    return rName;
}

TextureManager::TextureManager()
{
    std::memset(_boundTextures, 0, MAX_TEXTURES);
}

void TextureManager::registerTexture(const char *textureSource, const std::string &texName)
{
    _textures.insert_or_assign(texName, Texture2D(textureSource));
}

bool TextureManager::textureRegistered(const TextureIdentifier &texName) const
{
    return _textures.contains(texName);
}

void TextureManager::allocateTexture(const std::string &texName)
{
    const auto texture = _textures.find(texName);
    if (texture == _textures.end())
        return;

    texture->second.allocateTexture();
}

int TextureManager::bindTexture(const std::string &texName)
{
    const auto texture = _textures.find(texName);
    if (texture == _textures.end())
        return -1;
    if (!texture->second.isAllocated())
        return -1;
    if (int location = isTextureBound(texture->second); location != -1)
        return location;
    if (_numBoundTextures == MAX_TEXTURES)
        return -1;

    for (int q = 0; q < MAX_TEXTURES; ++q)
    {
        if (_boundTextures[q] == 0)
        {
            _boundTextures[q] = texture->second;
            ++_numBoundTextures;
            glActiveTexture(GL_TEXTURE0 + q);
            glBindTexture(GL_TEXTURE_2D, texture->second);
            return q;
        }
    }

    assert(false); // to catch cases in debug when the engine can't support more shaders

    return -1;
}

std::tuple<int, int, int> TextureManager::bindMaterial(const Material &mat)
{
    return {bindTexture(mat.diffTextureName), bindTexture(mat.specTextureName), bindTexture(mat.emissionTextureName)};
}

void TextureManager::allocateMaterial(const Material &mat)
{
    allocateTexture(mat.diffTextureName);
    allocateTexture(mat.specTextureName);
    allocateTexture(mat.emissionTextureName);
}

void TextureManager::unbindTexture(int textureId)
{
    if (textureId >= MAX_TEXTURES || textureId < 0)
        return;
    if (_numBoundTextures == 0)
        return;
    if (_boundTextures[textureId] == 0)
        return;

    _boundTextures[textureId] = 0;
    --_numBoundTextures;
    glActiveTexture(GL_TEXTURE0 + textureId);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureManager::unbindAllTextures()
{
    for (int v = 0; v < MAX_TEXTURES; ++v)
    {
        glActiveTexture(GL_TEXTURE0 + v);
        glBindTexture(GL_TEXTURE_2D, 0);
        _boundTextures[v] = 0;
    }

    _numBoundTextures = 0;
}

void TextureManager::deallocateTexture(const std::string &texName)
{
    const auto texture = _textures.find(texName);
    if (texture == _textures.end())
        return;

    assert(isTextureBound(texture->second));
    if (isTextureBound(texture->second))
        return;

    texture->second.deallocateTexture();
}

void TextureManager::unregisterTexture(const std::string &texName)
{
    const auto texture = _textures.find(texName);
    if (texture == _textures.end())
        return;

    assert(isTextureBound(texture->second) != -1);
    if (isTextureBound(texture->second) != -1)
        return;

    _textures.erase(texName);
}

void TextureManager::cleanUpGracefully()
{
    for (int f = 0; f < MAX_TEXTURES; ++f)
    {
        if (_boundTextures[f] == 0)
            continue;
        glActiveTexture(GL_TEXTURE0 + f);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    _textures.clear();
}

int TextureManager::isTextureBound(const Texture2D &texture)
{
    if (!texture.isAllocated())
        return -1;

    for (int b = 0; b < MAX_TEXTURES; ++b)
    {
        if (_boundTextures[b] == texture)
            return b;
    }

    return -1;
}
