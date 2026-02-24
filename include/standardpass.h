#pragma once

#include "types.h"
#include "framepass.h"
#include "instancedshader.h"

class WorldPlaneShader;
class LightVisualizationShader;
class SkyboxShader;
class PbrShader;

class StandardPass : public FramePass
{
public:
    void runPass() override;

private:
    uint32_t _hdrFrameBuffer = 0;
};
