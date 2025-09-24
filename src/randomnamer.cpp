#include "randomnamer.h"

#include <iostream>

RandomNamer::RandomNamer() : _charDist(50, 255)
{
    std::random_device rd;
    _generator = std::mt19937(rd());
}

RandomNamer *RandomNamer::instance()
{
    static RandomNamer instance;
    return &instance;
}

std::string RandomNamer::getRandomName(size_t nameLength)
{
    std::string rName;
    rName.resize(nameLength);
    for (int a = 0; a < nameLength; ++a)
    {
        const auto c = _charDist(_generator);
        rName.push_back(_charDist(_generator));
    }
    std::cout << "New name: " << rName << std::endl;

    return rName;
}
