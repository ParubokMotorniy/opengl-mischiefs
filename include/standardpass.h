#pragma once

#include "camera.h"
#include "framepass.h"

class StandardPass : public FramePass
{
public:
    void runPass() override;
    void setCamera(const Camera *newCamera);

private:
    const Camera *_currentViewCamera = nullptr;
};
