#pragma once

#include "material.h"
#include "mesh.h"
#include "types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <unordered_set>

struct ComponentPairHash
{
    std::size_t operator()(const Component &p) const noexcept
    {
        return std::hash<std::underlying_type_t<ComponentType>>{}(
            static_cast<std::underlying_type_t<ComponentType>>(p.first));
    }
};

struct ComponentPairEqual
{
    bool operator()(const Component &lhs, const Component &rhs) const noexcept
    {
        return lhs.first == rhs.first;
    }
};

struct GameObject
{
public:
    friend class ObjectManager;

    operator GameObjectIdentifier() const { return _objectId; }

    void addComponent(Component newComponent, bool overwrite = false)
    {
        if (overwrite)
        {
            _objectComponents.erase(newComponent);
        }
        _objectComponents.emplace(newComponent);
    }

    void addChildObject(GameObjectIdentifier newChild)
    {
        if (std::ranges::find(_objectChildren, newChild) != _objectChildren.end())
            return;
        _objectChildren.emplace_back(newChild);
    }

    ComponentIdentifier getIdentifierForComponent(ComponentType type) const
    {
        auto cPtr = _objectComponents.find(Component(type, InvalidIdentifier));
        return cPtr == _objectComponents.end() ? InvalidIdentifier : cPtr->second;
    }

    const std::vector<GameObjectIdentifier> &children() const { return _objectChildren; }

private:
    GameObject(GameObjectIdentifier newId) : _objectId(newId) {}

private:
    GameObjectIdentifier _objectId;
    std::unordered_set<Component, ComponentPairHash, ComponentPairEqual> _objectComponents;
    std::vector<GameObjectIdentifier> _objectChildren;
};
