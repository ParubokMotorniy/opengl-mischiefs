#pragma once

class Camera;
class Window;

class FramePass
{
public:
    virtual ~FramePass() = default;
    virtual void runPass() = 0;
    virtual void setCamera(const Camera *camPtr) { _currentCamera = camPtr; };
    virtual void setWindow(const Window *windowPtr) { _currentWindow = windowPtr; };

protected:
    const Camera *_currentCamera = nullptr;
    const Window *_currentWindow = nullptr;
};
