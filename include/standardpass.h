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
    StandardPass(InstancedBlinnPhongShader *ins, WorldPlaneShader *wrld, LightVisualizationShader *lightVis,
                 SkyboxShader *skybox, PbrShader *pbrShader);
    void runPass() override;

private:
    // TODO: ideally, these shouldn't be set by name from constructor
    InstancedBlinnPhongShader *_shaderProgramMain = nullptr;

    WorldPlaneShader *_worldPlaneShader = nullptr;

    LightVisualizationShader *_lightVisualizationShader = nullptr;

    SkyboxShader *_mainSkybox = nullptr;

    PbrShader *_pbrShader = nullptr;

    uint32_t _hdrFrameBuffer = 0;
};
