#pragma once

class FramePass
{
public:
    virtual ~FramePass() = default;
    virtual void runPass() = 0;
};
