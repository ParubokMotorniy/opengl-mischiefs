#pragma once

#include "framepass.h"

class TransparentShader;
class Camera;
class FullscreenFogShader;

class SortingTransparentPass : public FramePass
{
public:
    SortingTransparentPass(TransparentShader *transparentShader, FullscreenFogShader *fogShader);
    void runPass() override;
    void setCamera(const Camera *currentCamera) override;

private:
    TransparentShader *_transparentShader = nullptr;
    FullscreenFogShader *_fogShader = nullptr;
    const Camera *_currentCamera = nullptr;
};
