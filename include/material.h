#pragma once

#include <string>

//TODO: add manager for materials

struct Material
{
    std::string diffTextureName;
    std::string specTextureSampler;
    std::string emissionTextureSampler;
};
