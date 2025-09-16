#pragma once

#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>

#include <GLFW/glfw3.h>

class QuaternionCamera : public Camera
{
public:
    QuaternionCamera(glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH, float roll = ROLL) : Camera(position, up, yaw, pitch), _cameraRotation(glm::vec3(glm::radians(pitch), glm::radians(yaw), glm::radians(roll)))
    {
        updateCameraVectors();
    }

    virtual glm::mat4 getViewMatrix()
    {
        return glm::lookAt(_position, _position + _front, _up);
    }

    virtual void processKeyboard(MovementInput keysPressed, float deltaTime) override
    {
        if (keysPressed.isEmpty())
            return;

        const auto projector = [](const glm::vec3 &input)
        { return glm::normalize(glm::vec3(input.x, 0.0f, input.z)); };

        glm::vec3 movementVector(0.0f);
        if (keysPressed.Forward == 1)
            movementVector += projector(_front);
        if (keysPressed.Backward == 1)
            movementVector -= projector(_front);
        if (keysPressed.Right == 1)
            movementVector += projector(_right);
        if (keysPressed.Left == 1)
            movementVector -= projector(_right);
        if (keysPressed.Up == 1)
            movementVector += _worldUp;
        if (keysPressed.Down == 1)
            movementVector -= _worldUp;

        assert(!glm::isnan(movementVector.x) && !glm::isnan(movementVector.y) && !glm::isnan(movementVector.z));

        float velocity = _movementSpeed * deltaTime;
        _position += glm::normalize(movementVector) * velocity;
    }

    virtual void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) override
    {
        xoffset *= _mouseSensitivity;
        yoffset *= _mouseSensitivity;
        
        const glm::quat yQDelta = glm::angleAxis(glm::radians(xoffset), _worldUp);
        _cameraRotation = glm::normalize(yQDelta * _cameraRotation);
        updateCameraVectors();

        const glm::quat xQDelta = glm::angleAxis(glm::radians(yoffset), _right);
        _cameraRotation = glm::normalize(xQDelta * _cameraRotation);
        updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        _up = glm::normalize(_cameraRotation * glm::vec3(0.0f, 1.0f, 0.0f));
        _front = glm::normalize(_cameraRotation * glm::vec3(0.0f, 0.0f, 1.0f));
        _right = glm::normalize(glm::cross(_front, _up));

        if(glm::abs(_right.y) > 1.0e-5)
        {
            std::cout << "Correcting the roll!" << std::endl;
            _right = glm::normalize(glm::vec3(-glm::sign(_front.z), 0.0f,-glm::sign(_front.z)*(-_front.x/_front.z)));
            _up = glm::normalize(glm::cross(_right, _front));
        }
    }

private:
    glm::quat _cameraRotation;
};
