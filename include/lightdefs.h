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

    //allows to set the intrinsic values of the light with intuitive degrees
    void computeIntrinsics(float inCutOffDeg, float outCutOffDeg)
    {
        const auto inCutOffRad = glm::radians(inCutOffDeg);
        const auto outCutOffRad = glm::radians(outCutOffDeg);

        innerCutOff = glm::cos(inCutOffRad);
        outerCutOff = glm::cos(outCutOffRad);
    }
};

struct TexturedSpotLight
{
    glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    float pad1 = 0;

    glm::vec3 specular = glm::vec3(0.0f, 0.0f, 0.0f);
    float innerCutOff = 0;

    float outerCutOff = 0;
    float attenuationConstantTerm;
    float attenuationLinearTerm;
    float attenuationQuadraticTerm;

    glm::vec3 dummyPosition;
    float pad3;

    glm::vec3 dummyDirection;
    float pad4;

    glm::mat4 lightView;
    glm::mat4 lightProj;

    GLuint64 textureIdx = 0;

    //allows to set the intrinsic values of the light with intuitive degrees
    void computeIntrinsics(float inCutOffDeg, float outCutOffDeg, TransformIdentifier tId)
    {
        const auto inCutOffRad = glm::radians(inCutOffDeg);
        const auto outCutOffRad = glm::radians(outCutOffDeg);

        innerCutOff = glm::cos(inCutOffRad);
        outerCutOff = glm::cos(outCutOffRad);

        setTransform(tId);
    }

    void setTransform(TransformIdentifier tId)
    {
        const Transform *t = TransformManager::instance()->getTransform(tId); 

        glm::vec3 front = t->rotation()
                          * glm::vec4(0.0, 0.0, 1.0f, 1.0f);
        glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, front));

        dummyPosition = t->position();
        dummyDirection = front;

        lightView = glm::lookAt(dummyPosition, dummyPosition + front, up);
        lightProj = glm::perspective(2 * glm::acos(outerCutOff), 1.0f, 0.1f, 100.0f);
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
