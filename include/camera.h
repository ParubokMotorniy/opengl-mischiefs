#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

struct KeyboardInput
{
    uint8_t Forward : 1 = 0;
    uint8_t Backward : 1 = 0;
    uint8_t Right : 1 = 0;
    uint8_t Left : 1 = 0;
    uint8_t Up : 1 = 0;
    uint8_t Down : 1 = 0;
    uint8_t PeekRight : 1 = 0;
    uint8_t PeekLeft : 1 = 0;

    bool motionIsZero() const
    {
        return fMotionIsZero() && rMotionIsZero() && uMotionIsZero();
    }

    bool fMotionIsZero() const
    {
        return (Forward - Backward) == 0;
    }

    bool rMotionIsZero() const
    {
        return (Right - Left) == 0;
    }

    bool uMotionIsZero() const
    {
        return (Up - Down) == 0;
    }
};

// TODO: remove this c++ heresy
const float YAW = 15.0f;
const float PITCH = 0.0f;
const float SPEED = 5.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float ROLL = 0.0f;

class Camera
{

    // TODO: make the parent camera use getter wrappers over the camera axes instead of raw values

public:
    Camera(glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : _front(glm::vec3(0.0f, 0.0f, -1.0f)), _movementSpeed(SPEED), _mouseSensitivity(SENSITIVITY), _zoom(ZOOM)
    {
        _position = position;
        _worldUp = up;
        _yaw = yaw;
        _pitch = pitch;
        updateCameraVectors();
    }

    virtual glm::mat4 getViewMatrix()
    {
        return glm::lookAt(_position, _position + _front, _up);
    }

    virtual void lookAt(const glm::vec3 &target)
    {
        _front = glm::normalize(target - _position);
        _right = glm::normalize(glm::cross(_front, _worldUp));
        _up = glm::normalize(glm::cross(_right, _front));

        _yaw = glm::degrees(glm::acos(glm::dot(glm::normalize(glm::vec2(_front.x, _front.z)), glm::vec2(1.0f, 0.0f))));
        _pitch = glm::degrees(glm::asin(_front.y));
    }

    virtual void processKeyboard(const KeyboardInput &keysPressed, float deltaTime)
    {
        if (keysPressed.motionIsZero())
        {
            return;
        }

        const auto projector = [](const glm::vec3 &input)
        { return glm::vec3(input.x, 0.0f, input.z); };

        glm::vec3 movementVector(0.0f, 0.0f, 0.0f);

        if (!keysPressed.fMotionIsZero())
        {
            if (keysPressed.Forward == 1)
                movementVector += projector(_front);
            if (keysPressed.Backward == 1)
                movementVector -= projector(_front);
        }
        if (!keysPressed.rMotionIsZero())
        {
            if (keysPressed.Right == 1)
                movementVector += projector(_right);
            if (keysPressed.Left == 1)
                movementVector -= projector(_right);
        }
        if (!keysPressed.uMotionIsZero())
        {
            if (keysPressed.Up == 1)
                movementVector += _worldUp;
            if (keysPressed.Down == 1)
                movementVector -= _worldUp;
        }

        float velocity = _movementSpeed * deltaTime;
        movementVector = glm::normalize(movementVector) * velocity;

        assert(!glm::isnan(movementVector.x) && !glm::isnan(movementVector.y) && !glm::isnan(movementVector.z));

        _position += movementVector;
    }

    virtual void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
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

    virtual void processMouseScroll(float yoffset)
    {
        _zoom -= (float)yoffset;
        if (_zoom < 1.0f)
            _zoom = 1.0f;
        if (_zoom > 45.0f)
            _zoom = 45.0f;
    }

    virtual float zoom()
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

    // TODO: move camera to a higher precision internals for smoother motion
protected:
    glm::vec3 _position;
    glm::vec3 _front;
    glm::vec3 _up;
    glm::vec3 _right;
    glm::vec3 _worldUp;

    float _movementSpeed;
    float _mouseSensitivity;

private:
    float _zoom;
    float _yaw;
    float _pitch;
};
