#pragma once
 
#include "types.h"

// TODO: add manager for materials

struct BasicMaterial
{
    TextureIdentifier diffTextureName = 0;
    TextureIdentifier specTextureName = 0;
    TextureIdentifier emissionTextureName = 0;
};
