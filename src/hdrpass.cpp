#include "hdrpass.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "framebuffermanager.h"
#include "texturemanager.h"
#include "meshmanager.h"
#include "window.h"

#include <iostream>

namespace
{
const glm::mat4 planeModelToNdc = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f),
                                              glm::vec3(1.0f, 0.0f, 0.0f));
}

HdrPass::HdrPass(MeshIdentifier planeId) : _planeMeshId(planeId)
{
    // TODO: manage through texture manager
    glGenFramebuffers(1, &_hdrFb);
    // create floating point color buffer
    glGenTextures(1, &_colorBuf);
    glBindTexture(GL_TEXTURE_2D, _colorBuf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _colorTextureId = TextureManager::instance()->registerTexture(_colorBuf);
    // create depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1920, 1080);
    // attach buffers
    FrameBufferManager::instance()->bindFrameBuffer(GL_FRAMEBUFFER, _hdrFb);
    FrameBufferManager::instance()->bindColorTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D, _colorBuf);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuf, 0);
    FrameBufferManager::instance()->bindDepthBuffer(GL_FRAMEBUFFER, rboDepth);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }
    FrameBufferManager::instance()->unbindFrameBuffer(GL_FRAMEBUFFER);
}

void HdrPass::runPass()
{
    FrameBufferManager::instance()->popFrameBuffer();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _currentWindow->resetViewport();

    runShader();

    FrameBufferManager::instance()->pushFrameBuffer(_hdrFb);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void HdrPass::setWindow(const Window *currentWindow)
{
    _currentWindow = currentWindow;
}

void HdrPass::runShader()
{
    use();

    const auto [viewportX, viewportY] = _currentWindow->currentWindowDimensions();

    const int bindingLocation = TextureManager::instance()->bindTexture(_colorTextureId);
    MeshManager::instance()->allocateMesh(_planeMeshId);
    MeshManager::instance()->bindMesh(_planeMeshId);

    setFloat("exposure", 0.35f); // TODO: make exposure tunable through imgui
    setInt("hdrColorTexture", bindingLocation);
    setMatrix4("modelToNdc", planeModelToNdc);
    setFloat("viewportXScale", (float)viewportX/1920.0f);
    setFloat("viewportYScale", (float)viewportY/1080.0f);

    glDrawElements(GL_TRIANGLES, MeshManager::instance()->getMesh(_planeMeshId)->numIndices(),
                   GL_UNSIGNED_INT, 0);

    MeshManager::instance()->unbindMesh();
    TextureManager::instance()->unbindTexture(_colorTextureId);
}

void HdrPass::compileAndAttachNecessaryShaders(uint32_t id)
{
    if (_vertexShaderId == 0)
    {
        const std::string &vShaderCode = readShaderSource(_vertexPath);

        const char *vPtr = vShaderCode.c_str();

        _vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(_vertexShaderId, 1, &vPtr, NULL);
        compileShader(_vertexShaderId);
    }

    glAttachShader(id, _vertexShaderId);

    if (_fragmentShaderId == 0)
    {
        const std::string &fShaderCode = readShaderSource(_fragmentPath);

        const char *fPtr = fShaderCode.c_str();

        _fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(_fragmentShaderId, 1, &fPtr, NULL);
        compileShader(_fragmentShaderId);
    }

    glAttachShader(id, _fragmentShaderId);
}

void HdrPass::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}
