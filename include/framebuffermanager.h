#pragma once

#include "glad/glad.h"
#include "singleton.h"
#include <cstddef>

class FrameBufferManager : SystemSingleton<FrameBufferManager>
{
public:
    friend class SystemSingleton<FrameBufferManager>;

    void bindFrameBuffer(GLenum traget, GLuint frameBufferId, size_t viewPortWidth = 1920,
                         size_t viewPortHeight = 1080) const;
    void bindDepthTexture(GLenum target, GLenum textureType, GLuint texIdx) const;
    void unbindFrameBuffer(GLenum target) const noexcept;
};
