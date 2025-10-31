#pragma once

#include "camera.h"
#include "framepass.h"
#include "window.h"

class InstancedShader;
class WorldPlaneShader;
class LightVisualizationShader;
class SkyboxShader;

class StandardPass : public FramePass
{
public:
    StandardPass(InstancedShader *ins, WorldPlaneShader *wrld, LightVisualizationShader *lightVis,
                 SkyboxShader *skybox);
    void runPass() override;
    void setCamera(const Camera *newCamera);
    void setWindow(const Window *newWindow);

private:
    const Camera *_currentViewCamera = nullptr;
    const Window *_currentTargetWindow = nullptr;

    // TODO: ideally, these shouldn't be set by name from constructor
    InstancedShader *_shaderProgramMain = nullptr;

    WorldPlaneShader *_worldPlaneShader = nullptr;

    LightVisualizationShader *_lightVisualizationShader = nullptr;

    SkyboxShader *_mainSkybox = nullptr;
};
