#pragma once

#include "framepass.h"

class TransparentShader;
class Camera;
class FullscreenFogShader;

class SortingTransparentPass : public FramePass
{
public:
    void runPass() override;
    void setCamera(const Camera *currentCamera) override;

private:
    const Camera *_currentCamera = nullptr;
};
