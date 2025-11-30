#pragma once

#include "glad/glad.h"
#include "singleton.h"
#include <cstddef>
#include <stack>

class FrameBufferManager : public SystemSingleton<FrameBufferManager>
{
public:
    friend class SystemSingleton<FrameBufferManager>;

    void bindFrameBuffer(GLenum traget, GLuint frameBufferId, size_t viewPortWidth = 1920,
                         size_t viewPortHeight = 1080) const;

    // this api is for persistent changes of the framebuffer across passes
    void pushFrameBuffer(GLuint frameBufferId);
    void popFrameBuffer();

    // this api is for local manipulation with the frambuffer (e.g. shaodws)
    void bindDepthBuffer(GLenum target, GLuint depthBuffer) const;
    void unbindFrameBuffer(GLenum target) const noexcept;

    GLuint getCurrentFramebuffer();

    void bindDepthTexture(GLenum target, GLenum textureType, GLuint texIdx) const;
    void bindColorTexture(GLenum target, GLenum textureType, GLuint texIdx) const;
    void setColorMode(GLuint mode) const;
    void setReadMode(GLuint mode) const;

protected:
    FrameBufferManager();

private:
    std::stack<GLuint> _frameBuffersStack;
};
