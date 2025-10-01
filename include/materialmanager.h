#pragma once

#include "singleton.h"
#include "types.h"

#include <unordered_map>
#include <string>

template <typename MaterialStruct, ComponentType MaterialType>
class MaterialManager : public SystemSingleton<MaterialManager<MaterialStruct, MaterialType>>
{
public:
    friend class SystemSingleton<MaterialManager<MaterialStruct, MaterialType>>;
    using NamedMaterial = NamedComponent<MaterialStruct>;

    std::pair<std::string, MaterialIdentifier> registerMaterial(MaterialStruct &&newMaterial)
    {
        const auto rName = RandomNamer::instance()->getRandomName(10);
        const auto id = registerMaterial(std::move(newMaterial), rName);
        return std::make_pair(rName, id);
    }

    MaterialIdentifier registerMaterial(MaterialStruct &&newMaterial, const std::string &matName)
    {
        if (const MeshIdentifier mi = materialRegistered(matName); mi != InvalidIdentifier)
            return mi;

        _materials.emplace(++_identifiers, NamedMaterial{matName, std::move(newMaterial)});
        return _identifiers;
    }

    MaterialIdentifier materialRegistered(const std::string &matName) const
    {
        const auto matPtr = std::ranges::find_if(_materials, [&matName](const auto &pair)
                                                 { return pair.second.componentName == matName; });
        return matPtr == _materials.end() ? InvalidIdentifier : matPtr->first;
    }

    const MaterialStruct &getMaterial(MaterialIdentifier id)
    {
        assert(id != InvalidIdentifier);

        return _materials.at(id).componentData;
    }

private:
    MaterialManager() = default;

private:
    MaterialIdentifier _identifiers = InvalidIdentifier;

    std::unordered_map<MaterialIdentifier, NamedMaterial> _materials;
};
