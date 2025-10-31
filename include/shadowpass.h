#pragma once

#include "framepass.h"

class ShadowPass : public FramePass
{
public:
    void runPass() override;
};
