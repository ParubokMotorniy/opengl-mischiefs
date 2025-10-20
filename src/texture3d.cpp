#include "texture3d.h"

Texture3D::Texture3D(const std::array<const char *, 6> &cubemapPaths,
                     bool enableAnisotropicFiltering, Texture3DParameters params)
    : _params(params), _useAnisotropic(enableAnisotropicFiltering)
{
    for (int t = 0; t < 6; ++t)
    {
        _cubemapPaths[t] = std::string(cubemapPaths[t]);
    }
}

void Texture3D::allocateTexture()
{
    if (_textureId != 0)
        return;

    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _textureId);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(_cubemapPaths[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB,
                         GL_UNSIGNED_BYTE, data);
        }
    
        stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    // glGenTextures(1, &_textureId);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, _textureId);
    // for (int t = 0; t < 6; ++t)
    // {
    //     int width, height, numChannels;
    //     auto *imageData = stbi_load(_cubemapPaths[t].c_str(), &width, &height, &numChannels, 0);

    //     GLenum format;
    //     if (numChannels == 1)
    //         format = GL_RED;
    //     else if (numChannels == 3)
    //         format = GL_RGB;
    //     else if (numChannels == 4)
    //         format = GL_RGBA;

    //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + t, 0, format, width, height, 0, format,
    //                  GL_UNSIGNED_BYTE, imageData);

    //     stbi_image_free(imageData);
    // }

    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, _params.wrappingS);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, _params.wrappingT);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, _params.wrappingR);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, _params.filteringMin);
    // glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, _params.filteringMag);

    // if (_useAnisotropic)
    // {
    //     float maxAnisoLevel;
    //     glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisoLevel);

    //     glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT,
    //                     std::min(_anisoLevel, maxAnisoLevel));
    // }

    // glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture3D::deallocateTexture()
{
    if (_textureId == 0)
        return;
    if (glIsTextureHandleResidentARB(_textureId))
    {
        glMakeTextureHandleNonResidentARB(glGetTextureHandleARB(_textureId));
    }
    glDeleteTextures(1, &_textureId);
    _textureId = 0;
}

bool Texture3D::isAllocated() const noexcept { return _textureId != 0; }

void Texture3D::setUseAnisotropic(bool useAniso, size_t level)
{
    _useAnisotropic = useAniso;
    _anisoLevel = level;
}

void Texture3D::setParameters(Texture3DParameters params) { _params = params; }

Texture3D::~Texture3D() { deallocateTexture(); }