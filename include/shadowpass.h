#pragma once

#include "camera.h"
#include "framepass.h"
#include "window.h"

class InstancedShader;
class WorldPlaneShader;
class LightVisualizationShader;
class SkyboxShader;

class ShadowPass : public FramePass
{
public:
    ShadowPass(InstancedShader *ins, WorldPlaneShader *wrld, LightVisualizationShader *lightVis,
                 SkyboxShader *skybox);
    void runPass() override;

private:
    // TODO: ideally, these shouldn't be set by name from constructor
    InstancedShader *_shaderProgramMain = nullptr;

    WorldPlaneShader *_worldPlaneShader = nullptr;

    LightVisualizationShader *_lightVisualizationShader = nullptr;

    SkyboxShader *_mainSkybox = nullptr;
};
