#pragma once

#include "camera.h"
#include "framepass.h"
#include "passthroughshader.h"
#include "window.h"

class InstancedShader;
class WorldPlaneShader;
class LightVisualizationShader;
class SkyboxShader;

class ShadowPass : public FramePass
{
public:
    ShadowPass(InstancedShader *ins, LightVisualizationShader *lightVis);
    void runPass() override;

private:
    // TODO: ideally, these shouldn't be set by name from constructor
    InstancedShader *_shaderProgramMain = nullptr;
    LightVisualizationShader *_lightVisualizationShader = nullptr;
    PassThroughShader _passThroughOverride{ nullptr, nullptr };
};
