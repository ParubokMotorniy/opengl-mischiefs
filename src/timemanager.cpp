#include "timemanager.h"

#include <GLFW/glfw3.h>

void TimeManager::update()
{
    const float time = getTime();
    _deltaTime = time - _previousTime;
    _previousTime = time;
}

float TimeManager::getTime() const noexcept { return glfwGetTime(); }

float TimeManager::getDeltaTime() const noexcept { return _deltaTime; }
