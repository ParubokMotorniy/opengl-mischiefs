#pragma once

#include "glad/glad.h"

#include "framebuffermanager.h"
#include "lightdefs.h"
#include "randomnamer.h"
#include "singleton.h"
#include "texturemanager.h"
#include "transformmanager.h"
#include "types.h"

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <vector>

// TODO: consider adding buffer defragmentation instead of relying on upper bound
template <ComponentType LightType>
class LightManager : public SystemSingleton<LightManager<LightType>>
{
public:
    friend class SystemSingleton<LightManager<LightType>>;

    using LightStruct = decltype(getStructForLightType<LightType>());
    using LightComponent = std::pair<NamedComponent<LightStruct>, TransformIdentifier>;

public:
    size_t getMaxLights() { return MaxLights; }

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

        updateManager();
    }

    // returns frame_buffer_id + depth_texture_id
    static std::pair<GLuint, TextureIdentifier> createShadowMapPremises(size_t shadowResolutionX,
                                                                        size_t shadowResolutionY)
    {
        // TODO: add support for cubemaps
        unsigned int depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowResolutionX, shadowResolutionY, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

#ifdef ENGINE_ENABLE_HW_PCF
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#endif
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        unsigned int depthMapFBO;
        glGenFramebuffers(1, &depthMapFBO);

        FrameBufferManager::instance()->bindFrameBuffer(GL_FRAMEBUFFER, depthMapFBO);
        FrameBufferManager::instance()->bindDepthTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D, depthMap);
        FrameBufferManager::instance()->unbindFrameBuffer(GL_FRAMEBUFFER);

        const TextureIdentifier shadowIdentifier = TextureManager::instance()->registerTexture(
            depthMap);

        return { depthMapFBO, shadowIdentifier };
    }

    size_t getNumberOfBoundLights()
    {
        return _lastActiveLightIdx + 1;
    }

    void updateManager() // both boundBuffer and uniform buffer can be left fragmented after this
                         // operation
    {
        auto sourcesPtr = _lightComponents.begin();
        auto boundPtr = _boundSources.begin();

        while (boundPtr < _boundSources.cend())
        {
            const bool idValid = (*boundPtr != InvalidIdentifier);

            if (idValid && _sourceValidator(_lightComponents.at(*boundPtr).first.componentData))
                continue; // the light source is still valid (e.g. visible, enabled, affecting
                          // other
            // objects etc.)

            // the light source is not valid, it has to be replaced
            if (idValid)
            {
                updateLightSource(*boundPtr, LightStruct());
                *boundPtr = InvalidIdentifier;
                if ((boundPtr - _boundSources.cbegin())
                    == _lastActiveLightIdx) // we can only remove lights from the end.
                {
                    --_lastActiveLightIdx;
                }
            }

            // the buffer slot is free (!idValid) + there could be lights left that can be allocated
            while (sourcesPtr != _lightComponents.cend())
            {
                if (_sourceValidator(sourcesPtr->second.first.componentData)
                    && std::ranges::find(_boundSources, sourcesPtr->first) == _boundSources.cend())
                {
                    const LightSourceIdentifier lId = sourcesPtr->first;
                    // add a new light source to the uniform buffer
                    *boundPtr = lId;
                    updateLightSource(lId, sourcesPtr->second.first.componentData);
                    _lastActiveLightIdx = boundPtr - _boundSources.cbegin();
                    break;
                }
                ++sourcesPtr;
            }

            ++boundPtr;
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
        newLight.setTransform(_lightComponents.at(*lightPtr).second);

        _lightComponents.at(*lightPtr).first.componentData = newLight;

        glBindBuffer(GL_UNIFORM_BUFFER, _lBufferId);
        glBufferSubData(GL_UNIFORM_BUFFER, lightIdx * sizeof(LightStruct), sizeof(LightStruct),
                        &newLight);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void updateLightSourceTransform(LightSourceIdentifier lId)
    {
        if (_lBufferId == 0)
            return;

        auto lightPtr = std::ranges::find(_boundSources, lId);
        if (lightPtr == _boundSources.end())
            return;

        const size_t lightIdx = lightPtr - _boundSources.cbegin();
        auto &light = _lightComponents.at(*lightPtr);

        light.first.componentData.setTransform(light.second);

        glBindBuffer(GL_UNIFORM_BUFFER, _lBufferId);
        glBufferSubData(GL_UNIFORM_BUFFER, lightIdx * sizeof(LightStruct), sizeof(LightStruct),
                        &light.first.componentData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void bindLightBuffer(size_t lightBindingLocation)
    {
        if (_lBufferId == 0)
            return;

        glBindBufferBase(GL_UNIFORM_BUFFER, lightBindingLocation, _lBufferId);
    }

    LightSourceIdentifier registerNewLight(const std::string &name, TransformIdentifier lightTId)
    {
        return _lightComponents
            .emplace(++_identifiers,
                     LightComponent{
                         NamedComponent<LightStruct>{
                             name.empty() ? RandomNamer::instance()->getRandomName(7) : name,
                             LightStruct() },
                         lightTId })
            .first->first;
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

    std::vector<LightStruct> getLights()
    {
        std::vector<LightStruct> target;
        target.reserve(_lightComponents.size());

        std::transform(_lightComponents.cbegin(), _lightComponents.cend(),
                       std::back_inserter(target),
                       [](const auto &lightPair) { return lightPair.second.first.componentData; });
        return target;
    }

protected:
    LightManager() { _boundSources.fill(InvalidIdentifier); }

private:
    constexpr static size_t MaxLights = getMaxLightsForLightType(LightType);

    LightSourceIdentifier _identifiers = InvalidIdentifier;
    std::map<LightSourceIdentifier, LightComponent> _lightComponents;

    std::array<LightSourceIdentifier, MaxLights> _boundSources;

    std::function<bool(LightStruct)>
        _sourceValidator; // since we could have many light sources, not all will fit into the fixed
                          // uniform buffers. This thing tells whether a light source can be evicted
                          // from the buffer

    int _lastActiveLightIdx = -1; // tells where in the buffer the last active light is

    GLuint _lBufferId = 0;
};
