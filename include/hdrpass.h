#pragma once

#include "framepass.h"
#include "shaderprogram.h"
#include "types.h"

class Window;

class HdrPass : public FramePass, public ShaderProgram
{
public:
    HdrPass(MeshIdentifier planeId);
    void runPass() override;
    void setWindow(const Window *currentWindow);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    GLuint _hdrFb = 0;
    GLuint _colorBuf = 0;
    TextureIdentifier _hdrColorTexture = InvalidIdentifier;

    const char *_vertexPath = ENGINE_SHADERS "/hdr.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/hdr.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
    TextureIdentifier _colorTextureId = InvalidIdentifier;

    MeshIdentifier _planeMeshId = InvalidIdentifier;
    const Window *_currentWindow = nullptr;
};