#pragma once

#include "glad/glad.h"
#include "stb_image.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct Texture3DParameters
{
    uint32_t wrappingS = 0;
    uint32_t wrappingT = 0;
    uint32_t wrappingR = 0;

    uint32_t filteringMin = 0;
    uint32_t filteringMag = 0;
};

class CubemapManager;
class Cubemap
{
    friend class CubemapManager;

public:
    static constexpr int NumberOfFaces = 6;
    void setUseAnisotropic(bool useAniso, size_t level);
    void setParameters(Texture3DParameters params);

    operator int() const { return _textureId; }

private:
    explicit Cubemap(const std::array<const char *, NumberOfFaces> &cubemapPaths,
                     bool enableAnisotropicFiltering = false,
                     const Texture3DParameters &params = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                                                           GL_CLAMP_TO_EDGE, GL_LINEAR,
                                                           GL_LINEAR });
    explicit Cubemap(const std::array<std::string, NumberOfFaces> &cubemapPaths,
                     bool enableAnisotropicFiltering = false,
                     const Texture3DParameters &params = { GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
                                                           GL_CLAMP_TO_EDGE, GL_LINEAR,
                                                           GL_LINEAR });
    void allocateTexture();

    void deallocateTexture();

    bool isAllocated() const noexcept;

private:
    uint32_t _textureId = 0;
    std::array<std::string, NumberOfFaces> _cubemapPaths;
    Texture3DParameters _params;

    bool _useAnisotropic = false;
    float _anisoLevel = 8.0f;
};
