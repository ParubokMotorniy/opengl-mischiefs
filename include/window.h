#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <functional>
#include <vector>
#include <variant>

struct KeyboardInput
{
    //TODO: probably turn it into an enum
    uint8_t Forward : 1 = 0;
    uint8_t Backward : 1 = 0;
    uint8_t Left : 1 = 0;
    uint8_t Right : 1 = 0;
    uint8_t Up : 1 = 0;
    uint8_t Down : 1 = 0;
    uint8_t PeekLeft : 1 = 0;
    uint8_t PeekRight : 1 = 0;
    uint8_t MouseLeft : 1 = 0;
    uint8_t MouseRight : 1 = 0;
    uint8_t CtrlLeft : 1 = 0;
    uint8_t CtrlRight : 1 = 0;

    bool motionIsZero() const
    {
        return fMotionIsZero() && rMotionIsZero() && uMotionIsZero();
    }

    bool fMotionIsZero() const
    {
        return (Forward - Backward) == 0;
    }

    bool rMotionIsZero() const
    {
        return (Right - Left) == 0;
    }

    bool uMotionIsZero() const
    {
        return (Up - Down) == 0;
    }
};

class Window
{
public:
    using KeyboardEventsListener = std::function<void(KeyboardInput, KeyboardInput, float)>; // pressed now; released, deltaTime
    using FrameBufferResizeEventsListener = std::function<void(int, int)>;

    struct MouseMotionDescriptor
    {
        float deltaPosX = 0;
        float deltaPosY = 0;
    };
    using MouseMotionEventsListener = std::function<void(KeyboardInput, MouseMotionDescriptor)>;

    struct ScrollDescriptor
    {
        float deltaScrollX = 0;
        float deltaScrollY = 0;
    };
    using ScrollEventsListener = std::function<void(KeyboardInput, ScrollDescriptor)>;

    Window(size_t widthX, size_t widthY, const char *windowName);
    bool shouldClose() const;
    GLFWwindow *getRawWindow() const;
    virtual void update(float deltaTime);
    void hideCursor(bool ifHide) const;
    void setMouseAccuracy(bool accurate) const;

    template <typename ListenerType>
    void subscribeEventListener(ListenerType &&listener)
    {
        _eventListeners.emplace_back(EventListener(std::move(listener)));
    }

private:
    void processInput();

private:
    KeyboardInput _lastInput;
    KeyboardInput _releasedKeys;

    float _lastMouseDeltaX = 0;
    float _lastMouseDeltaY = 0;
    bool _dirtyMouseDelta = false;

    float _lastScrollDeltaX = 0;
    float _lastScrollDeltaY = 0;
    bool _dirtyScrollDelta = false;

    float _lastViewportWidth = 0;
    float _lastViewportHeight = 0;
    bool _dirtyViewportDelta = false;

protected:
    using EventListener = std::variant<KeyboardEventsListener, FrameBufferResizeEventsListener, MouseMotionEventsListener, ScrollEventsListener>;
    std::vector<EventListener> _eventListeners;

    GLFWwindow *_window = nullptr;

    friend void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    friend void mouseCallback(GLFWwindow *window, double xposIn, double yposIn);
    friend void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
};
