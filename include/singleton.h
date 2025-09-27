#pragma once

template<typename SubClass>
class SystemSingleton 
{
public:
    static SubClass *instance()
    {
        static SubClass instance;
        return &instance;
    }

    SystemSingleton(const SystemSingleton &other) = delete;
    SystemSingleton(SystemSingleton &&other) = delete;

    SystemSingleton& operator=(const SystemSingleton &other) = delete;
    SystemSingleton& operator=(SystemSingleton &&other) = delete;
protected: //so that children can be constructed
    SystemSingleton() = default;
};
