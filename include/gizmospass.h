#pragma once

#include "framepass.h"

class GeometryShaderProgram;

class GizmosPass : public FramePass
{
public:
    GizmosPass(GeometryShaderProgram *axesShader);
    void runPass() override;

private:
    GeometryShaderProgram *_axesShader = nullptr;
};
