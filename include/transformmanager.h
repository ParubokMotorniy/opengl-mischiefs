#pragma once

#include "singleton.h"
#include "types.h"

#include "glm/glm.hpp"

#include <unordered_map>

struct Transform
{
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::mat4 rotation = glm::identity<glm::mat4>();
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    bool dirty = false; // TODO: make use of this when propagating transformations to children

    glm::mat4 computeModelMatrix() const
    {
        glm::mat4 modelM = glm::identity<glm::mat4>();
        modelM = glm::scale(modelM, scale);
        modelM = rotation * modelM;
        modelM = glm::translate(modelM, position);

        return modelM;
    }

    glm::mat4 computeModelMatrixNoScale() const
    {
        glm::mat4 modelM = glm::identity<glm::mat4>();
        modelM = rotation * modelM;
        modelM = glm::translate(modelM, position);

        return modelM;
    }
};

class TransformManager : public SystemSingleton<TransformManager>
{
public:
    friend class SystemSingleton;

    TransformIdentifier registerNewTransform()
    {
        _transforms.emplace(++_identifiers, Transform{});
        return _identifiers;
    }

    glm::mat4 getModelMatrix(TransformIdentifier id)
    {
        assert(_transforms.contains(id)); // this should never occur in production

        return _transforms.at(id).computeModelMatrix();
    }

    // TODO: add general-purpose in-editor stuff like moving objects around, scaling and rotating them etc...

private:
    TransformManager() = default;

private:
    TransformIdentifier _identifiers = 0;
    std::unordered_map<TransformIdentifier, Transform> _transforms;
};
