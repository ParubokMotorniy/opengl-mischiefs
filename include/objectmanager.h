#pragma once

#include "singleton.h"
#include "types.h"
#include "object.h"

#include <vector>
#include <memory>

class ObjectManager : public SystemSingleton<ObjectManager>
{
public:
    friend class SystemSingleton;

    GameObjectIdentifier addObject()
    {
        _gameObjects.emplace_back(new GameObject(++_identifiers));
        return _identifiers;
    }

    GameObject &getObject(GameObjectIdentifier id)
    {
        assert(id != InvalidIdentifier);

        return *_gameObjects[id - 1];
    }

private:
    ObjectManager() = default;

private:
    GameObjectIdentifier _identifiers = 0;
    std::vector<std::unique_ptr<GameObject>> _gameObjects = {}; // TODO: vector here for faster access. May want to replace it with some sparse structure in the future
};
