#pragma once

#include <string>
#include <random>

class RandomNamer
{
public:
    static RandomNamer *instance();
    [[nodiscard]] std::string getRandomName(size_t nameLength);

    RandomNamer(const RandomNamer &other) = delete;
    RandomNamer(RandomNamer &&other) = delete;

    RandomNamer &operator=(const RandomNamer &other) = delete;
    RandomNamer &operator=(RandomNamer &&other) = delete;

private:
    RandomNamer();

private:
    std::mt19937 _generator;
    std::uniform_int_distribution<uint8_t> _charDist; 
};
