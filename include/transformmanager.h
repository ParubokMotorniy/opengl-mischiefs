#pragma once

#include "singleton.h"
#include "types.h"

#include "glm/glm.hpp"

#include <unordered_map>

class TransformManager;

struct Transform
{
    friend class TransformManager;

    glm::mat4 computeModelMatrix() const
    {
        glm::mat4 modelM = glm::identity<glm::mat4>();
        glm::mat4 scaleMat = {{_scale.x, 0.0f, 0.0f, 0.0f}, {0.0f, _scale.y, 0.0f, 0.0f}, {0.0f, 0.0f, _scale.z, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};
        glm::mat4 transMat = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {_position.x, _position.y, _position.z, 1.0f}};
        modelM = transMat * _rotation * scaleMat * modelM;

        return modelM;
    }

    glm::mat4 computeModelMatrixNoScale() const
    {
        glm::mat4 modelM = glm::identity<glm::mat4>();
        glm::mat4 transMat = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}, {_position.x, _position.y, _position.z, 1.0f}};
        modelM = transMat * _rotation * modelM;

        return modelM;
    }

    glm::vec3 scale() const { return _scale; };
    glm::mat4 rotation() const { return _rotation; };
    glm::vec3 position() const { return _position; };

    void setScale(const glm::vec3 &newScale)
    {
        _scale = newScale;
        dirty = true;
    }

    void setPosition(const glm::vec3 &newPosition)
    {
        _position = newPosition;
        dirty = true;
    }

    void setRotation(const glm::mat4 &newRotation)
    {
        _rotation = newRotation;
        dirty = true;
    }

private:
    glm::vec3 _scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::mat4 _rotation = glm::identity<glm::mat4>();
    glm::vec3 _position = glm::vec3(0.0f, 0.0f, 0.0f);

    bool dirty = false; // TODO: make use of this when propagating transformations to children
    Transform() = default;
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

    Transform *getTransform(TransformIdentifier tId)
    {
        auto t = _transforms.find(tId);

        return t == _transforms.end() ? nullptr : (&t->second);
    }

private:
    TransformManager() = default;

private:
    TransformIdentifier _identifiers = 0;
    std::unordered_map<TransformIdentifier, Transform> _transforms;
};
