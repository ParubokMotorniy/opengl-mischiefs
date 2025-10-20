#pragma once

#include "glad/glad.h"
#include "stb_image.h"

#include <array>
#include <cstdint>
#include <string>

struct Texture3DParameters
{
    uint32_t wrappingS = 0;
    uint32_t wrappingT = 0;
    uint32_t wrappingR = 0;

    uint32_t filteringMin = 0;
    uint32_t filteringMag = 0;
};

class TextureManager3D;
class Texture3D
{
    friend class TextureManager3D;

public:
    void setUseAnisotropic(bool useAniso, size_t level);
    void setParameters(Texture3DParameters params);

    operator int() const { return _textureId; }

    ~Texture3D();

private:
    Texture3D(const std::array<const char *, 6> &cubemapPaths,
              bool enableAnisotropicFiltering = false,
              Texture3DParameters params = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                                             GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR });
    void allocateTexture();

    void deallocateTexture();

    bool isAllocated() const noexcept;

private:
    uint32_t _textureId = 0;
    std::array<std::string, 6> _cubemapPaths;
    Texture3DParameters _params;

    bool _useAnisotropic = false;
    float _anisoLevel = 8.0f;
};
