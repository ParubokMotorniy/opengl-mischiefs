#pragma once

#include "object.h"
#include "singleton.h"
#include "types.h"

#include <memory>
#include <vector>

class ObjectManager : public SystemSingleton<ObjectManager>
{
public:
    friend class SystemSingleton;

    GameObjectIdentifier addObject()
    {
        _gameObjects.emplace_back(new GameObject(++_identifiers));
        return _identifiers;
    }

    GameObjectIdentifier copyObject(GameObjectIdentifier gTemplate)
    {
        GameObjectIdentifier parentCopy = copyBase(gTemplate);
        for (const GameObjectIdentifier gId : getObject(gTemplate).children())
        {
            copyObject(gId, parentCopy);
        }
        return parentCopy;
    }

    GameObject &getObject(GameObjectIdentifier id)
    {
        assert(id != InvalidIdentifier);

        return *_gameObjects[id - 1];
    }

private:
    GameObjectIdentifier copyBase(GameObjectIdentifier oldMe)
    {
        GameObject newMe = getObject(addObject());
        newMe._objectComponents = getObject(oldMe)._objectComponents;
        newMe.addComponent(Component(ComponentType::TRANSFORM,
                                     TransformManager::instance()->registerNewTransform()),
                           true);
        return newMe;
    }

    void copyObject(GameObjectIdentifier oldMe, GameObjectIdentifier newParent)
    {
        GameObjectIdentifier newMe = copyBase(oldMe);
        getObject(newParent).addChildObject(newMe);

        for (const GameObjectIdentifier cId : getObject(oldMe).children())
        {
            copyObject(cId, newMe);
        }
    }

    ObjectManager() = default;

private:
    GameObjectIdentifier _identifiers = 0;
    std::vector<std::unique_ptr<GameObject>> _gameObjects
        = {}; // TODO: vector here for faster access. May want to replace it with some sparse
              // structure in the future
};
