#pragma once

#include "glm/glm.hpp"

#include "transformmanager.h"
#include "types.h"

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
        dummyDirection = TransformManager::instance()->getTransform(tId)->rotation()
                         * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
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
        dummyDirection = TransformManager::instance()->getTransform(tId)->rotation()
                         * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        dummyPosition = TransformManager::instance()->getTransform(tId)->position();
    }
};

struct TexturedSpotLight
{
    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float lightSpan = 0;
    
    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float innerCutOff = 0;
    
    float outerCutOff = 0;
    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;
    
    glm::vec3 dummyPosition;
    float pad3;
    
    glm::vec3 boundVectorX = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad5;
    
    glm::vec3 boundVectorY = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad6;
    
    glm::mat4 dummyRotation;

    GLuint64 textureIdx = 0;
    // glm::mat4 computeRotation()
    // {
    //     return glm::mat4{ { dummyDirection.z, 0.0f, -dummyDirection.y, 0.0f },
    //                       { 0.0f, 1.0f, 0.0f, 0.0f },
    //                       { dummyDirection.y, 0.0f, dummyDirection.z, 0.0f },
    //                       { 0.0f, 0.0f, 0.0f, 1.0f } }
    //            * glm::mat4{ { 1.0f, 0.0f, 0.0f, 0.0f },
    //                         { 0.0f, dummyDirection.z, dummyDirection.y, 0.0f },
    //                         { 0.0f, -dummyDirection.y, dummyDirection.z, 0.0f },
    //                         { 0.0f, 0.0f, 0.0f, 1.0f } }
    //            * glm::mat4{ { dummyDirection.x, dummyDirection.y, 0.0f, 0.0f },
    //                         { -dummyDirection.y, dummyDirection.x, 0.0f, 0.0f },
    //                         { 0.0f, 0.0f, 1.0f, 0.0f },
    //                         { 0.0f, 0.0f, 0.0f, 1.0f } };
    // }

    void computeIntrinsics(float inCutOffDeg, float outCutOffDeg)
    {
        const auto inCutOffRad = glm::radians(inCutOffDeg);
        const auto outCutOffRad = glm::radians(outCutOffDeg);

        innerCutOff = glm::cos(inCutOffRad);
        outerCutOff = glm::cos(outCutOffRad);
        lightSpan = 2 * outCutOffRad;

        boundVectorX = glm::vec3(glm::tan(outCutOffRad), 0.0f, 1.0f);
        boundVectorY = glm::vec3(0.0f, glm::tan(outCutOffRad), 1.0f);
    }

    void setTransform(TransformIdentifier tId)
    {
        dummyRotation = TransformManager::instance()->getTransform(tId)->rotation();
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
    case ComponentType::LIGHT_TEXTURED_SPOT:
        return 4;
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
    if constexpr (cType == ComponentType::LIGHT_TEXTURED_SPOT)
        return TexturedSpotLight();
}
