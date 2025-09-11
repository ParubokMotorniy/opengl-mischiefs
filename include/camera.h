#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

struct MovementInput
{
    uint8_t Forward : 1;
    uint8_t Backward : 1;
    uint8_t _right : 1;
    uint8_t Left : 1;
    uint8_t _up : 1;
    uint8_t Down : 1;

    bool isEmpty() const
    {
        return Forward + Backward + _right + Left + _up + Down == 0;
    }
};

const float YAW = 15.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    Camera(glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : _front(glm::vec3(0.0f, 0.0f, -1.0f)), _movementSpeed(SPEED), _mouseSensitivity(SENSITIVITY), _zoom(ZOOM)
    {
        _position = position;
        _worldUp = up;
        _yaw = yaw;
        _pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(_position, _position + _front, _up);
    }

    void lookAt(const glm::vec3 &target)
    {
        _front = target - _position;
        _right = glm::normalize(glm::cross(_front, _worldUp));
        _up = glm::normalize(glm::cross(_right, _front));
    }

    void processKeyboard(MovementInput keysPressed, float deltaTime)
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
        if (keysPressed._right == 1)
            movementVector += projector(_right);
        if (keysPressed.Left == 1)
            movementVector -= projector(_right);
        if (keysPressed._up == 1)
            movementVector += _worldUp;
        if (keysPressed.Down == 1)
            movementVector -= _worldUp;

        float velocity = _movementSpeed * deltaTime;
        _position += glm::normalize(movementVector) * velocity;
    }

    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= _mouseSensitivity;
        yoffset *= _mouseSensitivity;

        _yaw += xoffset;
        _pitch += yoffset;

        if (constrainPitch)
        {
            if (_pitch > 89.0f)
                _pitch = 89.0f;
            if (_pitch < -89.0f)
                _pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void processMouseScroll(float yoffset)
    {
        _zoom -= (float)yoffset;
        if (_zoom < 1.0f)
            _zoom = 1.0f;
        if (_zoom > 45.0f)
            _zoom = 45.0f;
    }

    float zoom()
    {
        return _zoom;
    }

    glm::vec3 position()
    {
        return _position;
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front = glm::normalize(front);
        _right = glm::normalize(glm::cross(_front, _worldUp));
        _up = glm::normalize(glm::cross(_right, _front));
    }

private:
    glm::vec3 _position;
    glm::vec3 _front;
    glm::vec3 _up;
    glm::vec3 _right;
    glm::vec3 _worldUp;

    float _yaw;
    float _pitch;

    float _movementSpeed;
    float _mouseSensitivity;
    float _zoom;
};
