#pragma once

#include "stb_image.h"

#include <string>
#include <cstdint>

class Texture2D
{
public:
    Texture2D(const char *textureSourcePath);
    void allocateTexture();

    void deallocateTexture();

    bool isAllocated() const noexcept;

    operator int() const
    {
        return _textureId;
    }

    ~Texture2D();

private:
    uint32_t _textureId = 0;
    const char *_textureSourcePath = nullptr;
};
