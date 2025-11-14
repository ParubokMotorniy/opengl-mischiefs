#pragma once

#include "singleton.h"

class TimeManager : public SystemSingleton<TimeManager>
{
public:
    friend class SystemSingleton<TimeManager>;
    void update();
    [[nodiscard]] float getTime() const noexcept;
    [[nodiscard]] float getDeltaTime() const noexcept;

private:
    float _deltaTime{ 0.0f };
    float _previousTime{ 0.0f };
};
