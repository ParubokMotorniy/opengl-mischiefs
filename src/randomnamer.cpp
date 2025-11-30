#include "randomnamer.h"

#include <iostream>

RandomNamer::RandomNamer() : _charDist(64, 90)
{
    std::random_device rd;
    _generator = std::mt19937(rd());
}

std::string RandomNamer::getRandomName(size_t nameLength)
{
    std::string rName;
    rName.resize(nameLength);
    for (int a = 0; a < nameLength; ++a)
    {
        rName.push_back(_charDist(_generator));
    }  

    //TODO: add decent logging subsystem
    std::cout << "New random name: " << rName << std::endl;

    return rName;
}
