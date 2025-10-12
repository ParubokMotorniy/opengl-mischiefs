#include "transformmanager.h"

#include <cassert>
#include <ranges>

glm::mat4 Transform::computeModelMatrix() const
{
    glm::mat4 modelM = glm::identity<glm::mat4>();
    glm::mat4 scaleMat = { { _scale.x, 0.0f, 0.0f, 0.0f },
                           { 0.0f, _scale.y, 0.0f, 0.0f },
                           { 0.0f, 0.0f, _scale.z, 0.0f },
                           { 0.0f, 0.0f, 0.0f, 1.0f } };
    glm::mat4 transMat = { { 1.0f, 0.0f, 0.0f, 0.0f },
                           { 0.0f, 1.0f, 0.0f, 0.0f },
                           { 0.0f, 0.0f, 1.0f, 0.0f },
                           { _position.x, _position.y, _position.z, 1.0f } };
    modelM = transMat * _rotation * scaleMat * modelM;

    return modelM;
}

glm::mat4 Transform::computeModelMatrixNoScale() const
{
    glm::mat4 modelM = glm::identity<glm::mat4>();
    glm::mat4 transMat = { { 1.0f, 0.0f, 0.0f, 0.0f },
                           { 0.0f, 1.0f, 0.0f, 0.0f },
                           { 0.0f, 0.0f, 1.0f, 0.0f },
                           { _position.x, _position.y, _position.z, 1.0f } };
    modelM = transMat * _rotation * modelM;

    return modelM;
}

glm::vec3 Transform::scale() const { return _scale; }
glm::mat4 Transform::rotation() const { return _rotation; }
glm::vec3 Transform::position() const { return _position; }

void Transform::setScale(const glm::vec3 &newScale)
{
    _newScale = newScale;
    _dirty = true;
}

void Transform::setPosition(const glm::vec3 &newPosition)
{
    _newPosition = newPosition;
    _dirty = true;
}

void Transform::setRotation(const glm::mat4 &newRotation)
{
    _newRotation = newRotation;
    _dirty = true;
}

void Transform::propagateTransformUpdate(const glm::vec3 &parentDeltaScale,
                                         const glm::vec3 &parentDeltaPos,
                                         const glm::mat4 &parentDeltaRot)
{
    const glm::vec3 effectiveDeltaScale = parentDeltaScale
                                          * (_dirty ? (_newScale / _scale)
                                                    : glm::vec3(1.0f, 1.0f, 1.0f));
    const glm::vec3 effectiveDeltaPosition = parentDeltaPos
                                             + (_dirty ? (_newPosition - _position)
                                                       : glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 effectiveDeltaRotation = parentDeltaRot
                                       * (_dirty ? glm::mat4(glm::mat3(_newRotation)
                                                             * glm::transpose(glm::mat3(_rotation)))
                                                 : glm::identity<glm::mat4>());

    effectiveDeltaRotation[3][3] = 1.0f;
    normalizeMatrix(effectiveDeltaRotation);

    _dirty = false;

    _scale = effectiveDeltaScale * _scale;
    _position = _position + effectiveDeltaPosition;
    _rotation = effectiveDeltaRotation * _rotation;
    normalizeMatrix(_rotation);

    std::ranges::for_each(TransformManager::instance()->getChildTransforms(_parentId),
                          [=](TransformIdentifier tId) {
                              TransformManager::instance()
                                  ->getTransform(tId)
                                  ->propagateTransformUpdate(effectiveDeltaScale,
                                                             effectiveDeltaPosition,
                                                             effectiveDeltaRotation);
                          });
}

void Transform::normalizeMatrix(glm::mat4 &target) const
{
    glm::mat4 orthoTarget;
    orthoTarget[0] = glm::normalize(target[0]);
    orthoTarget[1] = glm::normalize(target[1]
                                    - glm::dot(orthoTarget[0], target[1]) * orthoTarget[0]);
    orthoTarget[2] = glm::normalize(target[2] - glm::dot(orthoTarget[1], target[2]) * orthoTarget[1]
                                    - glm::dot(orthoTarget[0], target[2]) * orthoTarget[0]);
    orthoTarget[3] = { 0.0f, 0.0f, 0.0f, 1.0f };

    target = orthoTarget;
}

Transform::Transform(GameObjectIdentifier parent) : _parentId(parent) {}

TransformIdentifier TransformManager::registerNewTransform(GameObjectIdentifier parentId)
{
    _transforms.emplace(++_identifiers, Transform(parentId));
    return _identifiers;
}

Transform *TransformManager::getTransform(TransformIdentifier tId)
{
    auto t = _transforms.find(tId);
    return t == _transforms.end() ? nullptr : (&(t->second));
}

std::vector<TransformIdentifier> TransformManager::getChildTransforms(GameObjectIdentifier parentId)
{
    GameObject &parent = ObjectManager::instance()->getObject(parentId);
    std::vector<TransformIdentifier> childrenTransforms;
    childrenTransforms.reserve(parent.children().size());

    std::ranges::for_each(parent.children(), [&](GameObjectIdentifier gId) {
        childrenTransforms.emplace_back(
            ObjectManager::instance()->getObject(gId).getIdentifierForComponent(
                ComponentType::TRANSFORM));
    });

    return childrenTransforms;
}

void TransformManager::flushUpdates()
{
    std::ranges::for_each(_transforms, [this](const auto &pair) {
        Transform *t = getTransform(pair.first);
        if (t->_dirty)
        {
            t->propagateTransformUpdate(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                        glm::identity<glm::mat4>());
        }
    });
}

TransformManager::TransformManager() = default;
