#include "objectmanager.h"

#include <cassert>

GameObjectIdentifier ObjectManager::addObject()
{
    _gameObjects.emplace_back(new GameObject(++_identifiers));
    return _identifiers;
}

GameObjectIdentifier ObjectManager::copyObject(GameObjectIdentifier gTemplate)
{
    GameObjectIdentifier parentCopy = copyBase(gTemplate);
    for (const GameObjectIdentifier gId : getObject(gTemplate).children())
    {
        copyObject(gId, parentCopy);
    }
    return parentCopy;
}

GameObject &ObjectManager::getObject(GameObjectIdentifier id)
{
    assert(id != InvalidIdentifier);

    return *_gameObjects[id - 1];
}

GameObjectIdentifier ObjectManager::copyBase(GameObjectIdentifier oldMe)
{
    GameObject &newMe = getObject(addObject());
    newMe._objectComponents = getObject(oldMe)._objectComponents;

    const TransformIdentifier newTransform = TransformManager::instance()->registerNewTransform(
        newMe);
    newMe.addComponent(Component(ComponentType::TRANSFORM, newTransform), true);

    const TransformIdentifier oldTransform = getObject(oldMe)
                                                 ._objectComponents
                                                 .find(Component(ComponentType::TRANSFORM,
                                                                 InvalidIdentifier))
                                                 ->second;

    *TransformManager::instance()->getTransform(newTransform)
        = *TransformManager::instance()->getTransform(
            oldTransform); // copies the properties of transforms

    return newMe;
}

void ObjectManager::copyObject(GameObjectIdentifier oldMe, GameObjectIdentifier newParent)
{
    GameObjectIdentifier newMe = copyBase(oldMe);
    getObject(newParent).addChildObject(newMe);

    for (const GameObjectIdentifier cId : getObject(oldMe).children())
    {
        copyObject(cId, newMe);
    }
}

ObjectManager::ObjectManager() = default;
