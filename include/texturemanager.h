#pragma once

#include "texture.h"

#include "unordered_map"
#include "string"

#define MAX_TEXTURES 16

class TextureManager
{
public:
    static TextureManager *instance()
    {
        if (_instance == nullptr)
            _instance = new TextureManager();
        return _instance;
    }

    void registerTexture(const std::string &texName, const char *textureSource)
    {
        _textures.insert_or_assign(texName, Texture2D(textureSource));
    }

    void allocateTexture(const std::string &texName)
    {
        const auto texture = _textures.find(texName);
        if (texture == _textures.end())
            return;

        texture->second.allocateTexture();
    }

    int bindTexture(const std::string &texName)
    {
        const auto texture = _textures.find(texName);
        if (texture == _textures.end())
            return 0;
        if (!texture->second.isAllocated())
            return 0;
        if (_numBoundTextures == MAX_TEXTURES)
            return 0;
        if(int location = isTextureBound(texture->second); location != -1)
            return location;

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

        return 0;
    }

    void unbindTexture(int textureId)
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

    void unbindAllTextures()
    {
        for (int v = 0; v < MAX_TEXTURES; ++v)
        {
            glActiveTexture(GL_TEXTURE0 + v);
            glBindTexture(GL_TEXTURE_2D, 0);
            _boundTextures[v] = 0;
        }

        _numBoundTextures = 0;
    }

    void deallocateTexture(const std::string &texName)
    {
        const auto texture = _textures.find(texName);
        if (texture == _textures.end())
            return;

        assert(isTextureBound(texture->second));
        if (isTextureBound(texture->second))
            return;

        texture->second.deallocateTexture();
    }

    void unregisterTexture(const std::string &texName)
    {
        const auto texture = _textures.find(texName);
        if (texture == _textures.end())
            return;

        assert(isTextureBound(texture->second) != -1);
        if (isTextureBound(texture->second) != -1)
            return;

        _textures.erase(texName);
    }

    void cleanUpGracefully()
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

private:
    TextureManager()
    {
        std::memset(_boundTextures, 0, MAX_TEXTURES);
    }

    //returns -1 if is not bound and the binding index otherwise
    int isTextureBound(const Texture2D &texture)
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

private:
    static TextureManager *_instance;
    std::unordered_map<std::string, Texture2D> _textures;

    uint _boundTextures[MAX_TEXTURES];
    int _numBoundTextures = 0;
};
