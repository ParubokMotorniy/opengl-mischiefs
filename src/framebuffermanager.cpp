#include "framebuffermanager.h"

#include <cassert>

void FrameBufferManager::bindFrameBuffer(GLenum target, GLuint frameBufferId, size_t viewPortWidth,
                                         size_t viewPortHeight) const
{
    glViewport(0, 0, viewPortWidth, viewPortHeight);
    glBindFramebuffer(target, frameBufferId);
}

void FrameBufferManager::pushFrameBuffer(GLuint frameBufferId)
{
    _frameBuffersStack.push(frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
}

void FrameBufferManager::popFrameBuffer()
{
    if (_frameBuffersStack.size() > 1)
        _frameBuffersStack.pop();
    glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffersStack.top());
}

void FrameBufferManager::bindDepthTexture(GLenum target, GLenum textureType, GLuint texIdx) const
{
    glFramebufferTexture2D(target, GL_DEPTH_ATTACHMENT, textureType, texIdx, 0);
}

void FrameBufferManager::bindColorTexture(GLenum target, GLenum textureType, GLuint texIdx) const
{
    glFramebufferTexture2D(target, GL_COLOR_ATTACHMENT0, textureType, texIdx, 0);
}

void FrameBufferManager::setColorMode(GLuint mode) const { glDrawBuffer(mode); }

void FrameBufferManager::setReadMode(GLuint mode) const { glReadBuffer(mode); }

std::array<GLint, 4> FrameBufferManager::getViewportDims() const
{
    std::array<GLint, 4> dimensions;
    glGetIntegerv(GL_VIEWPORT, dimensions.data());
    return dimensions;
}

void FrameBufferManager::bindDepthBuffer(GLenum target, GLuint depthBuffer) const
{
    glFramebufferRenderbuffer(target, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
}

void FrameBufferManager::unbindFrameBuffer(GLenum target) const noexcept
{
    glBindFramebuffer(target, _frameBuffersStack.top());
}

GLuint FrameBufferManager::getCurrentFramebuffer()
{
    GLint drawFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    return drawFboId;
}

FrameBufferManager::FrameBufferManager()
{
    _frameBuffersStack.push(0); // pushes the default framebuffer to the bottom
}
