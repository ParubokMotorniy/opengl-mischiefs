#pragma once

#include <functional>

namespace Utilities
{
    struct ScopeGuard
    {
        ScopeGuard(std::function<void()> &&cleanupFunc) : cleanupFunction(std::move(cleanupFunc)) {}
        ~ScopeGuard() { cleanupFunction(); }

    private:
        std::function<void()> cleanupFunction;
    };
}
