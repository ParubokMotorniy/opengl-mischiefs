#include "texture3d.h"

Cubemap::Cubemap(const std::array<const char *, Cubemap::Cubemap::NumberOfFaces> &cubemapPaths,
                 bool enableAnisotropicFiltering, const Texture3DParameters &params)
    : _params(params), _useAnisotropic(enableAnisotropicFiltering)
{
    for (int t = 0; t < Cubemap::Cubemap::NumberOfFaces; ++t)
    {
        _cubemapPaths[t] = std::string(cubemapPaths[t]);
    }
}

Cubemap::Cubemap(const std::array<std::string, Cubemap::Cubemap::NumberOfFaces> &cubemapPaths,
                 bool enableAnisotropicFiltering, const Texture3DParameters &params)
    : _params(params), _useAnisotropic(enableAnisotropicFiltering)
{
    for (int t = 0; t < Cubemap::Cubemap::NumberOfFaces; ++t)
    {
        _cubemapPaths[t] = std::string(cubemapPaths[t]);
    }
}

void Cubemap::allocateTexture()
{
    if (_textureId != 0)
        return;

    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _textureId);

    int width, height, numChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(_cubemapPaths[i].c_str(), &width, &height, &numChannels, 0);
        if (data)
        {
            GLenum format = GL_RGBA;
            GLenum internalFormat;
            if (numChannels == 1)
            {
                internalFormat = GL_RED;
                format = GL_RED;
            }
            else if (numChannels == 3)
            {
                internalFormat = GL_SRGB;
                format = GL_RGB;
            }
            else if (numChannels == 4)
            {
                internalFormat = GL_SRGB_ALPHA;
                format = GL_RGBA;
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0,
                         format, GL_UNSIGNED_BYTE, data);
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, _params.wrappingS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, _params.wrappingT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, _params.wrappingR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, _params.filteringMin);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, _params.filteringMag);

    if (_useAnisotropic)
    {
        float maxAnisoLevel;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisoLevel);

        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        std::min(_anisoLevel, maxAnisoLevel));
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::deallocateTexture()
{
    if (_textureId == 0)
        return;
#if !ENGINE_DISABLE_BINDLESS_TEXTURES
    if (glIsTextureHandleResidentARB(glGetTextureHandleARB(_textureId)))
    {
        glMakeTextureHandleNonResidentARB(glGetTextureHandleARB(_textureId));
    }
#endif
    glDeleteTextures(1, &_textureId);
    _textureId = 0;
}

bool Cubemap::isAllocated() const noexcept { return _textureId != 0; }

void Cubemap::setUseAnisotropic(bool useAniso, size_t level)
{
    _useAnisotropic = useAniso;
    _anisoLevel = level;
}

void Cubemap::setParameters(Texture3DParameters params) { _params = params; }
