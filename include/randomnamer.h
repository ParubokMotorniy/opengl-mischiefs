#pragma once

#include "singleton.h"

#include <random>
#include <string>

class RandomNamer : public SystemSingleton<RandomNamer>
{
public:
    friend class SystemSingleton;

    [[nodiscard]] std::string getRandomName(size_t nameLength);

private:
    RandomNamer();

private:
    std::mt19937 _generator;
#ifdef LINUX
    std::uniform_int_distribution<uint8_t> _charDist;
#endif

#ifdef WINDOWS
    std::uniform_int_distribution<unsigned short> _charDist;
#endif
};
