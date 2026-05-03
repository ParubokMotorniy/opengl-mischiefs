#pragma once

#include "framepass.h"

class GeometryShaderProgram;

class GizmosPass : public FramePass
{
public:
    void runPass() override;
};
