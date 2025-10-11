#pragma once

#include "glad/glad.h"

#include "lightdefs.h"
#include "randomnamer.h"
#include "singleton.h"
#include "transformmanager.h"
#include "types.h"

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <vector>

// TODO: consider adding buffer defragmentation
template <ComponentType LightType>
class LightManager : public SystemSingleton<LightManager<LightType>>
{
public:
    friend class SystemSingleton<LightManager<LightType>>;

    using LightStruct = decltype(getStructForLightType<LightType>());
    using LightComponent = std::pair<NamedComponent<LightStruct>, TransformIdentifier>;

public:
    void setLightSourceValidator(std::function<bool(LightStruct)> &&newSourceValidator)
    {
        _sourceValidator = std::move(newSourceValidator);
    }

    void initializeLightBuffer()
    {
        glGenBuffers(1, &_lBufferId);
        glBindBuffer(GL_UNIFORM_BUFFER, _lBufferId);
        glBufferData(GL_UNIFORM_BUFFER, MaxLights * sizeof(LightStruct), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glGenBuffers(1, &_tBufferId);
        glBindBuffer(GL_UNIFORM_BUFFER, _tBufferId);
        glBufferData(GL_UNIFORM_BUFFER, MaxLights * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        updateManager();
    }

    void updateManager() // both boundBuffer and uniform buffer can be left fragmented after this
                         // operation
    {
        auto sourcesPtr = _lightComponents.cbegin();
        auto boundPtr = _boundSources.cbegin();

        while (boundPtr < _boundSources.cend())
        {
            const bool idValid = *boundPtr != InvalidIdentifier;

            if (idValid
                && _sourceValidator(_lightComponents.at(*boundPtr)->second.first.componentData))
                continue; // the light source is still valid (e.g. visible, enabled, affecting
                          // other
            // objects etc.)

            // the light source is not valid, it has to be replaced

            if (idValid)
            {
                updateLightSource(*boundPtr, LightStruct());
                *boundPtr = InvalidIdentifier;
            }

            while (sourcesPtr != _lightComponents.cend())
            {
                if (_sourceValidator(sourcesPtr->second.first.componentData)
                    && std::ranges::find(_boundSources, sourcesPtr->first) == _boundSources.cend())
                {
                    const LightSourceIdentifier lId = sourcesPtr->first;
                    // add a new light source to the uniform buffer
                    *boundPtr = lId;
                    updateLightSource(lId, sourcesPtr->second.first.componentData);
                }
                ++sourcesPtr;
            }
        }
    }

    void updateLightSource(LightSourceIdentifier lId, LightStruct newLight)
    {
        if (_lBufferId == 0)
            return;

        auto lightPtr = std::ranges::find(_boundSources, lId);
        if (lightPtr == _boundSources.end())
            return;

        const size_t lightIdx = lightPtr - _boundSources.cbegin();

        glBindBuffer(GL_UNIFORM_BUFFER, _lBufferId);
        glBufferSubData(GL_UNIFORM_BUFFER, lightIdx * sizeof(LightStruct), sizeof(LightStruct),
                        &newLight);

        glBindBuffer(GL_UNIFORM_BUFFER, _tBufferId);
        const glm::mat4 sourceModelMatrix = TransformManager::instance()
                                                ->getTransform(lightPtr->second.second)
                                                ->computeModelMatrixNoScale();
        glBufferSubData(GL_UNIFORM_BUFFER, lightIdx * sizeof(glm::mat4), sizeof(glm::mat4),
                        &sourceModelMatrix);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void bindLightBuffer(size_t lightBindingLocation, size_t transformBindingLocation)
    {
        if (_lBufferId == 0)
            return;

        if (_tBufferId == 0)
            return;

        if (lightBindingLocation == transformBindingLocation)
            return;

        glBindBufferBase(GL_UNIFORM_BUFFER, lightBindingLocation, _lBufferId);
        glBindBufferBase(GL_UNIFORM_BUFFER, transformBindingLocation, _tBufferId);
    }

    LightSourceIdentifier registerNewLight(const std::string &name, TransformIdentifier lightTId)
    {
        return _lightComponents
            .emplace(++_identifiers,
                     { lightTId,
                       { name.empty() ? RandomNamer::instance()->getRandomName(7) : name,
                         LightStruct() } })
            ->first;
    }
    std::string nameForLightSource(LightSourceIdentifier lId) const
    {
        auto lightPtr = _lightComponents.find(lId);

        return lightPtr == _lightComponents.end() ? std::string()
                                                  : lightPtr->second.first.componentName;
    }

    LightStruct *getLight(LightSourceIdentifier lId)
    {
        auto lightPtr = _lightComponents.find(lId);

        return lightPtr == _lightComponents.end() ? nullptr
                                                  : &(lightPtr->second.first.componentData);
    }

    void unregisterLightSource(LightSourceIdentifier lId)
    {
        if (std::ranges::find(_boundSources, lId) != _boundSources.cend())
            updateLightSource(lId, LightStruct());

        _lightComponents.erase(lId);
    }

protected:
    LightManager() { _boundSources.fill(InvalidIdentifier); }

private:
    constexpr static size_t MaxLights = getMaxLightsForLightType(LightType);

    LightSourceIdentifier _identifiers = InvalidIdentifier;
    std::map<LightSourceIdentifier, LightComponent> _lightComponents;

    std::array<LightSourceIdentifier, MaxLights> _boundSources;

    std::function<bool(LightStruct)> _sourceValidator;

    GLuint _lBufferId;
    GLuint _tBufferId;
};
