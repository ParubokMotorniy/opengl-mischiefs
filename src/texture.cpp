#include "texture.h"

#include "glad/glad.h"

#include <cmath>

Texture2D::Texture2D(const char *textureSourcePath, bool enableAnisotropicFiltering,
                     Texture2DParameters params)
    : _textureSourcePath(textureSourcePath),
      _params(params),
      _useAnisotropic(enableAnisotropicFiltering)
{
}

Texture2D::Texture2D(uint32_t textureId) : _textureId(textureId) {}

void Texture2D::allocateTexture()
{
    if (_textureId != 0)
        return;

    int width, height, numChannels;
    auto *imageData = stbi_load(_textureSourcePath.c_str(), &width, &height, &numChannels, 0);

    GLenum format;
    GLenum internalFormat;
    if (numChannels == 1)
    {
        format = GL_RED;
        internalFormat = GL_RED;
    }
    else if (numChannels == 3)
    {
        format = GL_RGB;
        internalFormat = GL_SRGB;
    }
    else if (numChannels == 4)
    {
        format = GL_RGBA;
        internalFormat = GL_SRGB_ALPHA;
    }

    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_2D, _textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _params.wrappingS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _params.wrappingT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _params.filteringMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _params.filteringMag);

    if (_useAnisotropic)
    {
        float maxAnisoLevel;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisoLevel);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        std::min(_anisoLevel, maxAnisoLevel));
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::deallocateTexture()
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

bool Texture2D::isAllocated() const noexcept { return _textureId != 0; }

void Texture2D::setUseAnisotropic(bool useAniso, size_t level)
{
    _useAnisotropic = useAniso;
    _anisoLevel = level;
}

void Texture2D::setParameters(Texture2DParameters params) { _params = params; }
