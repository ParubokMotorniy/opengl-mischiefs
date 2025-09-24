#include "texture.h"

#include "glad/glad.h"

Texture2D::Texture2D(const char *textureSourcePath) : _textureSourcePath(textureSourcePath) {}

void Texture2D::allocateTexture()
{
    if (_textureId != 0)
        return;

    int width, height, numChannels;
    auto *imageData = stbi_load(_textureSourcePath.c_str(), &width, &height, &numChannels, 0);

    GLenum format;
    if (numChannels == 1)
        format = GL_RED;
    else if (numChannels == 3)
        format = GL_RGB;
    else if (numChannels == 4)
        format = GL_RGBA;

    glGenTextures(1, &_textureId);
    glBindTexture(GL_TEXTURE_2D, _textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);
}

void Texture2D::deallocateTexture()
{
    if (_textureId = 0)
        return;
    glDeleteTextures(1, &_textureId);
    _textureId = 0;
}

bool Texture2D::isAllocated() const noexcept
{
    return _textureId != 0;
}

Texture2D::~Texture2D()
{
    deallocateTexture();
}
