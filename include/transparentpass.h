#pragma once

#include "framepass.h"

class TransparentShader;
class Camera;

class SortingTransparentPass : public FramePass
{
public:
    SortingTransparentPass(TransparentShader *_transparentShader);
    void runPass() override;
    void setCamera(const Camera *currentCamera) override;

private:
    TransparentShader *_transparentShader = nullptr;
    const Camera *_currentCamera = nullptr;
};
