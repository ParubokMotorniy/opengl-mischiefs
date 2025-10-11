#pragma once

#include "objectmanager.h"
#include "singleton.h"
#include "types.h"

#include "glm/glm.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

class TransformManager;

struct Transform
{
    friend class TransformManager;

public:
    glm::mat4 computeModelMatrix() const;
    glm::mat4 computeModelMatrixNoScale() const;

    glm::vec3 scale() const;
    glm::mat4 rotation() const;
    glm::vec3 position() const;

    void setScale(const glm::vec3 &newScale);
    void setPosition(const glm::vec3 &newPosition);
    void setRotation(const glm::mat4 &newRotation);

    Transform *opeartor(const Transform &other)
    {
        _scale = other._scale;
        _rotation = other._rotation;
        _position = other._position;

        _newScale = other._newScale;
        _newRotation = other._newRotation;
        _newPosition = other._newPosition;

        _dirty = other._dirty;

        return this;
    }

    Transform *opeartor(Transform &&other)
    {
        _scale = other._scale;
        _rotation = other._rotation;
        _position = other._position;

        _newScale = other._newScale;
        _newRotation = other._newRotation;
        _newPosition = other._newPosition;

        _dirty = other._dirty;

        return this;
    }

private:
    void propagateTransformUpdate(const glm::vec3 &parentDeltaScale,
                                  const glm::vec3 &parentDeltaPos, const glm::mat4 &parentDeltaRot);
    void normalizeMatrix(glm::mat4 &target) const;

    explicit Transform(GameObjectIdentifier parent);

private:
    glm::vec3 _scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::mat4 _rotation = glm::identity<glm::mat4>();
    glm::vec3 _position = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 _newScale = glm::vec3(1.0f);
    glm::mat4 _newRotation = glm::identity<glm::mat4>();
    glm::vec3 _newPosition = glm::vec3(0.0f);
    bool _dirty = false;

    GameObjectIdentifier _parentId;
};

class TransformManager : public SystemSingleton<TransformManager>
{
public:
    friend class SystemSingleton;

    TransformIdentifier registerNewTransform(GameObjectIdentifier parentId);
    Transform *getTransform(TransformIdentifier tId);
    std::vector<TransformIdentifier> getChildTransforms(GameObjectIdentifier parentId);
    void flushUpdates();

private:
    TransformManager();

private:
    TransformIdentifier _identifiers = 0;
    std::unordered_map<TransformIdentifier, Transform> _transforms;
};
