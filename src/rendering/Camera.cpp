#include "Camera.h"
#include <cmath>

Camera::Camera()
    : position(0.0f, 0.0f, 0.0f)
    , target(0.0f, 0.0f, -1.0f)
    , up(0.0f, 1.0f, 0.0f)
    , rotation(0.0f, 0.0f, 0.0f)
    , fovY(45.0f)
    , aspect(16.0f / 9.0f)
    , nearPlane(0.1f)
    , farPlane(100.0f)
    , viewMatrixDirty(true)
    , projectionMatrixDirty(true)
    , isPerspective(true)
{
}

void Camera::setPosition(const Vector3& position) {
    this->position = position;
    viewMatrixDirty = true;
}

void Camera::setTarget(const Vector3& target) {
    this->target = target;
    viewMatrixDirty = true;
}

void Camera::setUp(const Vector3& up) {
    this->up = up;
    viewMatrixDirty = true;
}

void Camera::move(const Vector3& movement) {
    Vector3 forward = getForward();
    Vector3 right = getRight();
    Vector3 worldUp = Vector3::up();
    
    // Transform movement from local space to world space
    Vector3 worldMovement = 
        right * movement.x + 
        worldUp * movement.y + 
        forward * movement.z;
    
    position += worldMovement;
    target += worldMovement;
    viewMatrixDirty = true;
}

void Camera::rotate(const Vector3& deltaRotation) {
    rotation += deltaRotation;
    
    // Clamp pitch to avoid gimbal lock
    rotation.x = std::max(-89.0f, std::min(89.0f, rotation.x));
    
    // Convert rotation to radians
    float pitchRad = rotation.x * 3.14159f / 180.0f;
    float yawRad = rotation.y * 3.14159f / 180.0f;
    
    // Calculate new forward direction
    Vector3 forward;
    forward.x = std::cos(yawRad) * std::cos(pitchRad);
    forward.y = std::sin(pitchRad);
    forward.z = std::sin(yawRad) * std::cos(pitchRad);
    forward.normalize();
    
    target = position + forward;
    viewMatrixDirty = true;
}

void Camera::lookAt(const Vector3& target) {
    this->target = target;
    
    // Update rotation to match the new target
    Vector3 direction = (target - position).normalized();
    rotation.x = std::asin(direction.y) * 180.0f / 3.14159f;
    rotation.y = std::atan2(direction.z, direction.x) * 180.0f / 3.14159f;
    
    viewMatrixDirty = true;
}

void Camera::setPerspective(float fovY, float aspect, float nearPlane, float farPlane) {
    this->fovY = fovY;
    this->aspect = aspect;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
    this->isPerspective = true;
    projectionMatrixDirty = true;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    // Store orthographic parameters in perspective variables for simplicity
    // In a full implementation, you'd have separate orthographic parameters
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
    this->isPerspective = false;
    projectionMatrixDirty = true;
}

void Camera::setAspectRatio(float aspect) {
    this->aspect = aspect;
    projectionMatrixDirty = true;
}

Matrix4 Camera::getViewMatrix() const {
    if (viewMatrixDirty) {
        updateViewMatrix();
        viewMatrixDirty = false;
    }
    return viewMatrix;
}

Matrix4 Camera::getProjectionMatrix() const {
    if (projectionMatrixDirty) {
        updateProjectionMatrix();
        projectionMatrixDirty = false;
    }
    return projectionMatrix;
}

Matrix4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

void Camera::updateViewMatrix() const {
    viewMatrix = Matrix4::lookAt(position, target, up);
}

void Camera::updateProjectionMatrix() const {
    if (isPerspective) {
        projectionMatrix = Matrix4::perspective(fovY, aspect, nearPlane, farPlane);
    } else {
        // Simplified orthographic projection - you'd implement proper ortho matrix here
        projectionMatrix = Matrix4::perspective(fovY, aspect, nearPlane, farPlane);
    }
}