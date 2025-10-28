#pragma once

#include "object.h"
#include "singleton.h"
#include "types.h"
#include "transformmanager.h"

#include <memory>
#include <vector>

class ObjectManager : public SystemSingleton<ObjectManager>
{
public:
    friend class SystemSingleton;

    GameObjectIdentifier addObject();
    GameObjectIdentifier copyObject(GameObjectIdentifier gTemplate);
    GameObject &getObject(GameObjectIdentifier id);

private:
    GameObjectIdentifier copyBase(GameObjectIdentifier oldMe);
    void copyObject(GameObjectIdentifier oldMe, GameObjectIdentifier newParent);

    ObjectManager();

private:
    GameObjectIdentifier _identifiers = 0;
    std::vector<std::unique_ptr<GameObject>> _gameObjects
        = {}; // TODO: vector here for faster access. May want to replace it with some sparse
              // structure in the future
};
