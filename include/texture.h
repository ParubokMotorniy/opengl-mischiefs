#pragma once

#include "glad/glad.h"
#include "stb_image.h"

#include <string>

class Texture2D
{
public:
    Texture2D(const char *textureSourcePath) : _textureSourcePath(textureSourcePath) {}
    void allocateTexture()
    {
        if (_textureId != 0)
            return;

        int width, height, numChannels;
        auto *imageData = stbi_load(_textureSourcePath, &width, &height, &numChannels, 0);

        glGenTextures(1, &_textureId);
        glBindTexture(GL_TEXTURE_2D, _textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(imageData);
    }

    void deallocateTexture()
    {
        if (_textureId = 0)
            return;
        glDeleteTextures(1, &_textureId);
        _textureId = 0;
    }

    bool isAllocated() const noexcept
    {
        return _textureId != 0;
    }

    operator int() const
    {
        return _textureId;
    }

    ~Texture2D()
    {
        deallocateTexture();
    }

private:
    uint _textureId = 0;
    const char *_textureSourcePath = nullptr;
};
