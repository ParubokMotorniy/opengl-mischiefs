#pragma once

#include <string>
#include <cstdint>
#include <utility>

using GameObjectIdentifier = uint64_t;
using ComponentIdentifier = uint64_t;

using TextureIdentifier = ComponentIdentifier;
using MeshIdentifier = ComponentIdentifier;
using MaterialIdentifier = ComponentIdentifier;
using TransformIdentifier = ComponentIdentifier;

constexpr ComponentIdentifier InvalidIdentifier = 0;

enum class ComponentType
{
    MESH,
    TRANSFORM,
    BASIC_MATERIAL,
};
using Component = std::pair<ComponentType, ComponentIdentifier>;

template <typename ComponentData>
struct NamedComponent
{
    std::string componentName;
    ComponentData componentData;
};
