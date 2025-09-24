#pragma once

#include "types.h"

// TODO: add manager for materials

struct Material
{
    TextureIdentifier diffTextureName = "";
    TextureIdentifier specTextureName = "";
    TextureIdentifier emissionTextureName = "";
};
