#pragma once

#include "singleton.h"

#include <string>
#include <random>

class RandomNamer : public SystemSingleton<RandomNamer>
{
public:
    friend class SystemSingleton;

    [[nodiscard]] std::string getRandomName(size_t nameLength);

private:
    RandomNamer();

private:
    std::mt19937 _generator;
    std::uniform_int_distribution<uint8_t> _charDist; 
};
