#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float camSpeed,
               float camSensitivity)
    : _front(glm::vec3(0.0f, 0.0f, -1.0f)),
      _movementSpeed(camSpeed),
      _mouseSensitivity(camSensitivity),
      _zoom(45.0f)
{
    _position = position;
    _worldUp = up;
    _yaw = yaw;
    _pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const { return glm::lookAt(_position, _position + _front, _up); }

void Camera::lookAt(const glm::vec3 &target)
{
    _front = glm::normalize(target - _position);
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));

    _yaw = glm::degrees(
        glm::acos(glm::dot(glm::normalize(glm::vec2(_front.x, _front.z)), glm::vec2(1.0f, 0.0f))));
    _pitch = glm::degrees(glm::asin(_front.y));
}

void Camera::moveTo(const glm::vec3 &pos) { _position = pos; }

void Camera::processKeyboard(const KeyboardInput &keysPressed, float deltaTime)
{
    if (keysPressed.motionIsZero())
    {
        return;
    }

    const auto projector = [](const glm::vec3 &input) { return glm::vec3(input.x, 0.0f, input.z); };

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

    assert(!glm::isnan(movementVector.x) && !glm::isnan(movementVector.y)
           && !glm::isnan(movementVector.z));

    _position += movementVector;
}

void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
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

void Camera::processMouseScroll(float yoffset)
{
    _zoom -= (float)yoffset;
    if (_zoom < 1.0f)
        _zoom = 1.0f;
    if (_zoom > 45.0f)
        _zoom = 45.0f;
}

float Camera::zoom() const { return _zoom; }

glm::vec3 Camera::position() const { return _position; }

void Camera::setProjectionMatrix(const glm::mat4 &newProjectionMatrix)
{
    _currentProjectionMatrix = newProjectionMatrix;
}

glm::mat4 Camera::projectionMatrix() const { return _currentProjectionMatrix; }

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    front.y = sin(glm::radians(_pitch));
    front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _front = glm::normalize(front);
    _right = glm::normalize(glm::cross(_front, _worldUp));
    _up = glm::normalize(glm::cross(_right, _front));
}
