#pragma once

#include "glad/glad.h"
#include "stb_image.h"

#include <array>
#include <cstdint>
#include <string>

struct Texture2DParameters
{
    uint32_t wrappingS = 0;
    uint32_t wrappingT = 0;

    uint32_t filteringMin = 0;
    uint32_t filteringMag = 0;
};

class TextureManager;
class Texture2D
{
    friend class TextureManager;

public:
    void setUseAnisotropic(bool useAniso, size_t level);
    void setParameters(Texture2DParameters params);

    operator int() const { return _textureId; }

private:
    explicit Texture2D(const char *textureSourcePath, bool enableAnisotropicFiltering = false,
              Texture2DParameters params = { GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT,
                                             GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR });
    explicit Texture2D(uint32_t textureId);

    void allocateTexture();

    void deallocateTexture();

    bool isAllocated() const noexcept;

private:
    uint32_t _textureId = 0;
    std::string _textureSourcePath;
    Texture2DParameters _params;

    bool _useAnisotropic = false;
    float _anisoLevel = 8.0f;
};
