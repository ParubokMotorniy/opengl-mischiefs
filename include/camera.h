#pragma once

#include "window.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cassert>
#include <cmath>

class Camera
{
    // TODO: make the parent camera use getter wrappers over the camera axes instead of raw values

public:
    Camera(glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = 15.0f, float pitch = 0.0f,
           float camSpeed = 10.0f, float camSensitivity = 0.12f);

    virtual glm::mat4 getViewMatrix() const;
    virtual void lookAt(const glm::vec3 &target);
    virtual void moveTo(const glm::vec3 &pos);
    virtual void processKeyboard(const KeyboardInput &keysPressed, float deltaTime);
    virtual void processMouseMovement(float xoffset, float yoffset,
                                      GLboolean constrainPitch = true);
    virtual void processMouseScroll(float yoffset);
    virtual float zoom() const;
    glm::vec3 position() const;
    void setProjectionMatrix(const glm::mat4 &newProjectionMatrix);
    glm::mat4 projectionMatrix() const;

private:
    void updateCameraVectors();

protected:
    glm::vec3 _position;
    glm::vec3 _front;
    glm::vec3 _up;
    glm::vec3 _right;
    glm::vec3 _worldUp;

    glm::mat4 _currentProjectionMatrix;

    float _movementSpeed;
    float _mouseSensitivity;

private:
    float _zoom;
    float _yaw;
    float _pitch;
};
