#include "texturemanager.h"
#include "randomnamer.h"

#include <glad/glad.h>

#include <algorithm>
#include <cassert>
#include <cstring>

std::pair<std::string, TextureIdentifier> TextureManager::registerTexture(const char *textureSource)
{
    const auto rName = RandomNamer::instance()->getRandomName(10);
    const auto id = registerTexture(textureSource, rName);
    return std::make_pair(rName, id);
}

TextureManager::TextureManager() { std::memset(_boundTextures, 0, MAX_TEXTURES); }

TextureIdentifier TextureManager::registerTexture(const char *textureSource,
                                                  const std::string &texName)
{
    if (const TextureIdentifier ti = textureRegistered(texName); ti != InvalidIdentifier)
        return ti;

    _textures.emplace(++_identifiers, NamedTexture{ texName, Texture2D(textureSource) });
    return _identifiers;
}

TextureIdentifier TextureManager::textureRegistered(const std::string &texName) const
{
    const auto texPtr = std::ranges::find_if(_textures, [&texName](const auto &pair) {
        return pair.second.componentName == texName;
    });
    return texPtr == _textures.end() ? InvalidIdentifier : texPtr->first;
}

void TextureManager::allocateTexture(TextureIdentifier id)
{
    const auto texture = _textures.find(id);
    if (texture == _textures.end())
        return;

    texture->second.componentData.allocateTexture();
}

int TextureManager::bindTexture(TextureIdentifier id)
{
    const auto texturePtr = _textures.find(id);
    if (texturePtr == _textures.end())
        return -1;
    const auto &texture = texturePtr->second.componentData;
    if (!texture.isAllocated())
        return -1;
    if (int location = isTextureBound(texture); location != -1)
        return location;
    if (_numBoundTextures == MAX_TEXTURES)
        return -1;

    for (int q = 0; q < MAX_TEXTURES; ++q)
    {
        if (_boundTextures[q] == 0)
        {
            _boundTextures[q] = texture;
            ++_numBoundTextures;
            glActiveTexture(GL_TEXTURE0 + q);
            glBindTexture(GL_TEXTURE_2D, texture);
            return q;
        }
    }

    assert(false); // to catch cases in debug when the engine can't support more shaders

    return -1;
}

std::tuple<int, int, int> TextureManager::bindMaterial(const BasicMaterial &mat)
{
    return { bindTexture(mat.diffTextureName), bindTexture(mat.specTextureName),
             bindTexture(mat.emissionTextureName) };
}

void TextureManager::allocateMaterial(const BasicMaterial &mat)
{
    allocateTexture(mat.diffTextureName);
    allocateTexture(mat.specTextureName);
    allocateTexture(mat.emissionTextureName);
}

void TextureManager::unbindTexture(TextureIdentifier id)
{
    const auto texturePtr = _textures.find(id);
    if (texturePtr == _textures.end())
        return;
    const int textureId = texturePtr->second.componentData;

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

void TextureManager::deallocateTexture(TextureIdentifier id)
{
    const auto texturePtr = _textures.find(id);
    if (texturePtr == _textures.end())
        return;
    auto &texture = texturePtr->second.componentData;

    assert(isTextureBound(texture));
    if (isTextureBound(texture))
        return;

    texture.deallocateTexture();
}

void TextureManager::unregisterTexture(TextureIdentifier id)
{
    const auto texturePtr = _textures.find(id);
    if (texturePtr == _textures.end())
        return;
    const auto &texture = texturePtr->second.componentData;

    assert(isTextureBound(texture) != -1);
    if (isTextureBound(texture) != -1)
        return;

    _textures.erase(id);
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

const Texture2D *TextureManager::getTexture(TextureIdentifier tId) const 
{ 
    const auto tPtr = _textures.find(tId);
    if(tPtr == _textures.end())
        return nullptr;

    return &tPtr->second.componentData;
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
