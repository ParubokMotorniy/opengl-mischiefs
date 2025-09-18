#include "window.h"

#include <iostream>

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_lastViewportWidth = width;
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_lastViewportHeight = height;
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_dirtyViewportDelta = true;
}

void mouseCallback(GLFWwindow *window, double xposIn, double yposIn)
{
    static float lastX{0.0f};
    static float lastY{0.0f};

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    static_cast<Window *>(glfwGetWindowUserPointer(window))->_lastMouseDeltaX = xoffset;
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_lastMouseDeltaY = yoffset;
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_dirtyMouseDelta = true;
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_lastScrollDeltaX = xoffset;
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_lastScrollDeltaY = yoffset;
    static_cast<Window *>(glfwGetWindowUserPointer(window))->_dirtyScrollDelta = true;
}

namespace
{
    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
}

void Window::processInput()
{
    auto *winPtr = _window;
    if (glfwGetKey(winPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(winPtr, true);

    // // // key ghosting does not allow some combinations of peek and diagonal motion :(
    _lastInput = KeyboardInput{
        glfwGetKey(winPtr, GLFW_KEY_W) == GLFW_PRESS,
        glfwGetKey(winPtr, GLFW_KEY_S) == GLFW_PRESS,
        glfwGetKey(winPtr, GLFW_KEY_D) == GLFW_PRESS,
        glfwGetKey(winPtr, GLFW_KEY_A) == GLFW_PRESS,
        glfwGetKey(winPtr, GLFW_KEY_SPACE) == GLFW_PRESS,
        glfwGetKey(winPtr, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS,
        glfwGetKey(winPtr, GLFW_KEY_E) == GLFW_PRESS,
        glfwGetKey(winPtr, GLFW_KEY_Q) == GLFW_PRESS,
        glfwGetMouseButton(winPtr, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS,
        glfwGetMouseButton(winPtr, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS};
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(_window);
}
GLFWwindow *Window::getRawWindow() const
{
    return _window;
}

void Window::update(float deltaTime)
{
    processInput();

    // TODO: remove repeated flag checking. It's slow.
    const auto visitor = overloaded{
        [this, deltaTime](const KeyboardEventsListener &listener) -> void
        { listener(_lastInput, deltaTime); },
        [this, deltaTime](const FrameBufferResizeEventsListener &listener) -> void
        { 
            if(!_dirtyViewportDelta)
                return;
            listener(_lastViewportWidth, _lastViewportHeight); },
        [this, deltaTime](const MouseMotionEventsListener &listener) -> void
        { 
            if(!_dirtyMouseDelta)
                return;
            listener(_lastInput, {_lastMouseDeltaX, _lastMouseDeltaY}); },
        [this, deltaTime](const ScrollEventsListener &listener)
        {
            if (!_dirtyScrollDelta)
                return;
            listener(_lastInput, {_lastScrollDeltaX, _lastScrollDeltaY});
        }};

    for (auto &listener : _eventListeners)
    {
        std::visit(visitor, listener);
    }

    {
        _dirtyMouseDelta = false;
        _dirtyScrollDelta = false;
        _dirtyViewportDelta = false;
    }
}

Window::Window(size_t widthX, size_t heightY, const char *windowName) : _lastViewportWidth(widthX), _lastViewportHeight(heightY)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    _window = glfwCreateWindow(widthX, heightY, windowName, NULL, NULL);
    if (_window == NULL)
    {
        std::cerr << "Window creation failed" << std::endl;
        _window = nullptr;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(_window);
    glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);
    glfwSetCursorPosCallback(_window, mouseCallback);
    glfwSetScrollCallback(_window, scrollCallback);
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetWindowUserPointer(_window, this);
}
