#pragma once

#include <functional>

namespace Utilities
{
struct ScopeGuard
{
    explicit ScopeGuard(std::function<void()> &&cleanupFunc)
        : cleanupFunction(std::move(cleanupFunc))
    {
    }
    ~ScopeGuard() { cleanupFunction(); }

private:
    std::function<void()> cleanupFunction;
};
} // namespace Utilities
