#pragma once

#include "glad/glad.h"

#include "objectmanager.h"
#include "randomnamer.h"
#include "singleton.h"
#include "texturemanager.h"
#include "types.h"

#include <array>
#include <concepts>
#include <iterator>
#include <string>
#include <type_traits>
#include <unordered_map>

template <typename MaterialStruct>
concept HasTextures = requires(MaterialStruct mS) {
    {
        mS.textures()
    } -> std::convertible_to<
        std::array<TextureIdentifier, std::tuple_size_v<decltype(mS.textures())>>>;
};

template <
    typename MaterialStruct, ComponentType MaterialType,
    size_t textureCount = std::tuple_size_v<decltype(std::declval<MaterialStruct>().textures())>>
    requires HasTextures<MaterialStruct>
class MaterialManager : public SystemSingleton<MaterialManager<MaterialStruct, MaterialType>>
{
public:
    friend class SystemSingleton<MaterialManager<MaterialStruct, MaterialType>>;
    using NamedMaterial = NamedComponent<MaterialStruct>;

    // TODO: I may want to compute hashes of the enclosed texure identifiers to check if the
    // porvided material exists already
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

        _materials.emplace(++_identifiers, NamedMaterial{ matName, std::move(newMaterial) });
        return _identifiers;
    }

    // for each of the provided objects, returns an array listing the indices for the textures
    // of its material in the uniform buffer of texture handles
    std::unordered_map<GameObjectIdentifier, std::array<int, textureCount>> bindTextures(
        const std::vector<GameObjectIdentifier> &objects, uint32_t bindingPoint,
        uint32_t &textureBufferIdx)
    {
        using TextureHandle = GLuint64;

        // run over all objects and determine the required textures + obtain handles
        std::unordered_map<TextureIdentifier, TextureHandle> texturesToHandles;
        std::ranges::for_each(objects, [&, this](GameObjectIdentifier gId) {
            const MaterialIdentifier mId
                = ObjectManager::instance()->getObject(gId).getIdentifierForComponent(MaterialType);

            if (mId == InvalidIdentifier)
                return; // some objects may not have materials on them

            const MaterialStruct &mS = getMaterial(mId);

            for (const TextureIdentifier tId : mS.textures())
            {
                if (tId == InvalidIdentifier || texturesToHandles.contains(tId))
                    continue;

                TextureManager::instance()->allocateTexture(tId);
#if ENGINE_DISABLE_BINDLESS_TEXTURES
                texturesToHandles.emplace(std::make_pair(tId, tId));
#else
                const TextureHandle texHandle = glGetTextureHandleARB(
                    *TextureManager::instance()->getTexture(tId)); //stuff in the parentheses obtains the gl texture id
                assert(texHandle != 0);

                texturesToHandles.emplace(std::make_pair(tId, texHandle));

                glMakeTextureHandleResidentARB(texHandle);
#endif
            }
        });

        // create an array of handles -> bind them
        std::vector<TextureHandle> uniformHandles;
        uniformHandles.reserve(texturesToHandles.size());

        std::unordered_map<TextureHandle, int> handleToIndex;
        handleToIndex.reserve(texturesToHandles.size());

        for (int m = 0; m < texturesToHandles.size(); ++m)
        {
            const TextureHandle handle = std::next(texturesToHandles.begin(), m)->second;
            uniformHandles.emplace_back(handle);
            handleToIndex.emplace(std::make_pair(handle, m));
        }

        glCreateBuffers(1, &textureBufferIdx);
        glNamedBufferStorage(textureBufferIdx, uniformHandles.size() * sizeof(TextureHandle),
                             (const void *)uniformHandles.data(), GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, textureBufferIdx);

        // run over all objects and for each determine the indices of the textures in the bound buffer
        std::unordered_map<GameObjectIdentifier, std::array<int, textureCount>> objectIndices;
        objectIndices.reserve(objects.size());

        std::ranges::for_each(objects, [&, this](GameObjectIdentifier gId) {
            const MaterialIdentifier mId
                = ObjectManager::instance()->getObject(gId).getIdentifierForComponent(MaterialType);
            std::array<int, textureCount> objArray;

            if (mId == InvalidIdentifier)
            {
                for (int g = 0; g < textureCount; ++g) // object has no material bound
                {
                    objArray[g] = -1;
                }
            }
            else
            {
                const MaterialStruct &mS = getMaterial(mId);

                const std::array<TextureIdentifier, textureCount> objTextures = mS.textures();

                for (int g = 0; g < textureCount; ++g)
                {
                    if (objTextures[g] == InvalidIdentifier)
                        objArray[g] = -1;
                    else
                        objArray[g] = handleToIndex[texturesToHandles[objTextures[g]]];
                }
            }

            objectIndices.emplace(std::make_pair(gId, objArray));
        });

        return objectIndices;
    }

    MaterialIdentifier materialRegistered(const std::string &matName) const
    {
        const auto matPtr = std::ranges::find_if(_materials, [&matName](const auto &pair) {
            return pair.second.componentName == matName;
        });
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
