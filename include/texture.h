#pragma once

#include "stb_image.h"
#include "glad/glad.h"

#include <cstdint>
#include <string>

struct Texture2DParameters
{
    uint32_t wrappingS = 0;
    uint32_t wrappingT = 0;

    uint32_t filteringMin = 0;
    uint32_t filteringMag = 0;
};

class Texture2D
{
public:
    Texture2D(const char *textureSourcePath,
              Texture2DParameters params = { GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT,
                                             GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR });
    void allocateTexture();

    void deallocateTexture();

    bool isAllocated() const noexcept;

    operator int() const { return _textureId; }

    ~Texture2D();

private:
    uint32_t _textureId = 0;
    std::string _textureSourcePath;
    Texture2DParameters _params;
};
