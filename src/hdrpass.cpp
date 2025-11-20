#include "hdrpass.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "imgui.h"

#include "framebuffermanager.h"
#include "meshmanager.h"
#include "texturemanager.h"
#include "window.h"

#include <iostream>

namespace
{
const glm::mat4 planeModelToNdc = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.0f),
                                              glm::vec3(1.0f, 0.0f, 0.0f));
bool useHdr = false;
bool useGamma = false;
float exposure = 0.3f;
} // namespace

HdrPass::HdrPass(MeshIdentifier planeId) : _planeMeshId(planeId)
{
    glGenFramebuffers(1, &_hdrFb);

    glGenTextures(1, &_colorBuf);
    glBindTexture(GL_TEXTURE_2D, _colorBuf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _colorTextureId = TextureManager::instance()->registerTexture(_colorBuf);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1920, 1080);

    FrameBufferManager::instance()->bindFrameBuffer(GL_FRAMEBUFFER, _hdrFb);
    FrameBufferManager::instance()->bindColorTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D, _colorBuf);
    FrameBufferManager::instance()->bindDepthBuffer(GL_FRAMEBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer not complete!" << std::endl;
    }
    FrameBufferManager::instance()->unbindFrameBuffer(GL_FRAMEBUFFER);
}

void HdrPass::runPass()
{
    {
        ImGui::Begin("HDR pass parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("HDR");
        ImGui::Separator();
        ImGui::Checkbox("Use HDR", &useHdr);
        ImGui::SliderFloat("Exposure", &exposure, 0.001f, 5.0f);

        ImGui::Separator();

        ImGui::Text("Gamma correction");
        ImGui::Separator();
        ImGui::Checkbox("Use gamma correction", &useGamma);

        ImGui::End();
    }

    FrameBufferManager::instance()->popFrameBuffer(); // remove hdr buffer from stack
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _currentWindow->resetViewport();

    runShader(); // render into the standard framebuffer

    FrameBufferManager::instance()->pushFrameBuffer(
        _hdrFb); // push the hdr buffer back onto the stack for later passes to render into
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void HdrPass::setWindow(const Window *currentWindow) { _currentWindow = currentWindow; }

void HdrPass::runShader()
{
    use();

    const auto [viewportX, viewportY] = _currentWindow->currentWindowDimensions();

    const int bindingLocation = TextureManager::instance()->bindTexture(_colorTextureId);
    MeshManager::instance()->allocateMesh(_planeMeshId);
    MeshManager::instance()->bindMesh(_planeMeshId);

    setMatrix4("modelToNdc", planeModelToNdc);

    setFloat("exposure", exposure);
    setFloat("gamma", useGamma ? 2.2f : 1.0f); //just to avoid branching in the shader
    setBool("useHdr", useHdr);
    setInt("hdrColorTexture", bindingLocation);
    setFloat("viewportXScale", (float)viewportX / 1920.0f);
    setFloat("viewportYScale", (float)viewportY / 1080.0f);

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
