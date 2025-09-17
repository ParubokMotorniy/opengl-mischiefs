#pragma once

#include "camera.h"
#include "window.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#include <optional>


class QuaternionCamera : public Camera
{
public:
    QuaternionCamera(glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float roll = ROLL) : Camera(position, up, yaw, pitch), _cameraRotation(glm::vec3(glm::radians(pitch), glm::radians(yaw), glm::radians(roll)))
    {
        updateCameraVectors();
    }

    virtual glm::mat4 getViewMatrix()
    {
        const auto roll = getRoll();

        return roll.has_value() ? glm::lookAt(_position, _position + _front, roll.value() * _up) : glm::lookAt(_position, _position + _front, _up);
    }

    virtual void lookAt(const glm::vec3 &target) override
    {
        const auto targetDir = glm::normalize(target - _position);
        const auto rotationAngle = glm::acos(glm::dot(_front, targetDir));

        const auto rotationAxis = glm::normalize(glm::cross(_front, targetDir));
        const auto targetRotation = glm::angleAxis(rotationAngle, rotationAxis);

        _cameraRotation = glm::normalize(targetRotation  * _cameraRotation);
        updateCameraVectors();
    }

    virtual void processKeyboard(const KeyboardInput &keysPressed, float deltaTime) override
    {
        const auto roll = getRoll();
        const auto cacheUp = _up;
        const auto cacheRight = _right;

        _up = roll.has_value() ? roll.value() * _up : _up;
        _right = roll.has_value() ? roll.value() * _right : _right;

        Camera::processKeyboard(keysPressed, deltaTime);

        _up = cacheUp;
        _right = cacheRight;

        _previousInput = keysPressed;
    }

    virtual void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) override
    {
        xoffset *= _mouseSensitivity;
        yoffset *= _mouseSensitivity;

        const glm::quat yQDelta = glm::angleAxis(glm::radians(-xoffset), _worldUp);
        _cameraRotation = glm::normalize(yQDelta * _cameraRotation);
        updateCameraVectors();

        const glm::quat xQDelta = glm::angleAxis(glm::radians(yoffset), _right);
        _cameraRotation = glm::normalize(xQDelta * _cameraRotation);
        updateCameraVectors();
    }

private:
    std::optional<glm::quat> getRoll()
    {
        if (_previousInput.PeekLeft == 1 ^ _previousInput.PeekRight == 1)
        {
            float peekAmount = 0.0f;

            if (_previousInput.PeekLeft == 1)
                peekAmount = -15.0f;
            else if (_previousInput.PeekRight == 1)
                peekAmount = 15.0f;

            return glm::angleAxis(glm::radians(peekAmount), _front);
        }
        return std::nullopt;
    }

    void updateCameraVectors()
    {
        _up = glm::normalize(_cameraRotation * glm::vec3(0.0f, 1.0f, 0.0f));
        _front = glm::normalize(_cameraRotation * glm::vec3(0.0f, 0.0f, 1.0f));
        _right = glm::normalize(glm::cross(_front, _up));

        // TODO: come up with a more sophisticated control system

        if (glm::abs(_right.y) > 1.0e-4) // this thing removes unintended roll buildup from the camera
        {
            _right = glm::normalize(glm::vec3(-glm::sign(_front.z), 0.0f, -glm::sign(_front.z) * (-_front.x / _front.z)));
            _up = glm::normalize(glm::cross(_right, _front));
        }
    }

private:
    glm::quat _cameraRotation;
    KeyboardInput _previousInput;
};
