#include "framebuffermanager.h"

#include <cassert>

void FrameBufferManager::bindFrameBuffer(GLenum target, GLuint frameBufferId, size_t viewPortWidth,
                                         size_t viewPortHeight) const
{
    glViewport(0, 0, viewPortWidth, viewPortHeight);
    glBindFramebuffer(target, frameBufferId);
    assert(glGetError == GL_NO_ERROR);
}

void FrameBufferManager::bindDepthTexture(GLenum target, GLenum textureType, GLuint texIdx) const
{
    glFramebufferTexture2D(target, GL_DEPTH_ATTACHMENT, textureType, texIdx, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

void FrameBufferManager::unbindFrameBuffer(GLenum target) const noexcept { glBindFramebuffer(target, 0); }
