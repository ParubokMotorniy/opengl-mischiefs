#pragma once

#include "object.h"
#include "meshmanager.h"
#include "texturemanager.h"

#include <vector>

struct Model
{
    std::vector<PrimitiveObject> modelComponents;

    void allocateModel()
    {
        for (const auto &component : modelComponents)
        {
            MeshManager::instance()->allocateMesh(component.objMesh);
            for (const auto &mat : component.objMaterials)
            {
                TextureManager::instance()->allocateMaterial(mat);
            }
        }
    }

    Model *operator+=(const Model &m1)
    {
        modelComponents.insert(modelComponents.end(), m1.modelComponents.begin(), m1.modelComponents.end());
        return this;
    }

    //TODO: make parent transform be applied instead of the children's
};
