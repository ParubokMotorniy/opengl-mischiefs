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
    QuaternionCamera(glm::vec3 position = glm::vec3(10.0f, 10.0f, 10.0f),
                     glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
                     float yaw = 15.0f,
                     float pitch = 0.0f,
                     float camSpeed  = 8.0f,
                     float camSensitivity = 0.12f);

    virtual glm::mat4 getViewMatrix() const override;
    virtual void lookAt(const glm::vec3 &target) override;
    virtual void processKeyboard(const KeyboardInput &keysPressed, float deltaTime) override;
    virtual void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) override;

private:
    std::optional<glm::quat> getRoll() const;
    void updateCameraVectors();

private:
    glm::quat _cameraRotation;
    KeyboardInput _previousInput;
};
