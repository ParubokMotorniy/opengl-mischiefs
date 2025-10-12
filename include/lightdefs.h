#pragma once

#include "glm/glm.hpp"

#include "types.h"
#include "transformmanager.h"

struct DirectionalLight
{
    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad2;

    glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad3;

    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad4;

    glm::vec3 dummyDirection;
    float pad1;

    void setTransform(TransformIdentifier tId)
    {
        dummyDirection = TransformManager::instance()->getTransform(tId)->rotation() * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f );
    }
};

struct PointLight
{
    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad2;

    glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad3;

    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float attenuationConstantTerm;
    
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;
    float pad4;
    float pad5;

    glm::vec3 dummyPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad1;

    void setTransform(TransformIdentifier tId)
    {
        dummyPosition = TransformManager::instance()->getTransform(tId)->position();
    }
};

struct SpotLight
{
    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad1;

    glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad2;

    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float innerCutOff;
    
    float outerCutOff;
    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;

    glm::vec3 dummyPosition;
    float pad3;

    glm::vec3 dummyDirection;
    float pad4;

    void setTransform(TransformIdentifier tId)
    {
        dummyDirection = TransformManager::instance()->getTransform(tId)->rotation() * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f );
        dummyPosition = TransformManager::instance()->getTransform(tId)->position();    
    }
};

constexpr size_t getMaxLightsForLightType(ComponentType lType)
{
    switch (lType)
    {
    case ComponentType::LIGHT_SPOT:
        return 16;
    case ComponentType::LIGHT_POINT:
        return 32;
    case ComponentType::LIGHT_DIRECTIONAL:
        return 8;
    default:
        return 0;
    }
}

template <ComponentType cType>
constexpr auto getStructForLightType()
{
    if constexpr (cType == ComponentType::LIGHT_POINT)
        return PointLight();
    if constexpr (cType == ComponentType::LIGHT_DIRECTIONAL)
        return DirectionalLight();
    if constexpr (cType == ComponentType::LIGHT_SPOT)
        return SpotLight();
}
