#pragma once

#include "glad/glad.h"
#include "stb_image.h"

#include <string>

class Texture2D
{
public:
    Texture2D(const char *textureSourcePath)
    {
        int width, height, numChannels;
        auto *imageData = stbi_load(textureSourcePath, &width, &height, &numChannels, 0);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(imageData);
    }
    operator int()
    {
        return textureId;
    }
    ~Texture2D()
    {
        glDeleteTextures(1, &textureId);
    }

private:
    uint textureId = 0;
};
