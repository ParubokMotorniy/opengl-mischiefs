#pragma once

#include "glm/glm.hpp"

#include "types.h"

struct DirectionalLight
{
    glm::vec3 direction;
    float pad1;

    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad2;

    glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad3;

    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad4;
};

struct PointLight
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad1;

    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad2;

    glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad3;

    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad4;

    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;
    float pad5;
};

struct SpotLight
{
    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad1;

    glm::vec3 diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad2;

    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad3;

    glm::vec3 position;
    float pad4;

    glm::vec3 direction;
    float pad5;

    float innerCutOff;
    float outerCutOff;
    float pad6;
    float pad7;
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
