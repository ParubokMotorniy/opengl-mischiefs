#pragma once

#include "framepass.h"
#include "instancedshader.h"
#include "passthroughshader.h"

class WorldPlaneShader;
class LightVisualizationShader;
class SkyboxShader;
class PbrShader;

class ShadowPass : public FramePass
{
public:
    ShadowPass(InstancedBlinnPhongShader *ins, LightVisualizationShader *lightVis,
               PbrShader *pbrShader);
    void runPass() override;

private:
    // TODO: ideally, these shouldn't be set by name from constructor
    InstancedBlinnPhongShader *_shaderProgramMain = nullptr;
    LightVisualizationShader *_lightVisualizationShader = nullptr;
    PbrShader *_pbrShader = nullptr;
    PassThroughShader _passThroughOverride{ nullptr, nullptr };
    PassThroughShader _passThroughOverridePbr{ nullptr, nullptr };
};
