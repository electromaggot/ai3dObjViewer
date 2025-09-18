#include "Camera.h"
#include <cmath>
#include <iostream>

Camera::Camera()
    : position(0.0f, 0.0f, 5.0f)  // Start back from origin
    , target(0.0f, 0.0f, 0.0f)    // Look at origin
    , up(0.0f, 1.0f, 0.0f)
    , rotation(0.0f, 0.0f, 0.0f)
    , fovY(45.0f)
    , aspect(16.0f / 9.0f)
    , nearPlane(0.1f)
    , farPlane(100.0f)
    , ortho(-5.0f, 5.0f, -5.0f, 5.0f)
    , orthoHeight(10.0f)
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

	/*CLAUDE_ADDED attempting to fix starting camera angle, but seems kludgey, doesn't work
				   anyway; so commented-out and rotation "hack" remains in Application.cpp.
    // Update rotation to match the new target direction
    Vector3 direction = (target - position).normalized();

    // Calculate pitch (rotation around X-axis)
    rotation.x = std::asin(direction.y) * 180.0f / 3.14159f;

    // Calculate yaw (rotation around Y-axis)
    // Note: atan2(z, x) gives angle in XZ plane from +X axis
    rotation.y = std::atan2(direction.z, direction.x) * 180.0f / 3.14159f;

    // Handle the case where direction.x ≈ 0 (looking along Z axis)
    if (std::abs(direction.x) < 0.0001f) {
        if (direction.z > 0) {
            rotation.y = 90.0f;  // Looking toward +Z
        } else {
            rotation.y = -90.0f; // Looking toward -Z
        }
    }
	CLAUDE_END*/

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

    // Update target based on new rotation
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
    this->ortho.left = left;
    this->ortho.right = right;
    this->ortho.bottom = bottom;
    this->ortho.top = top;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
    this->isPerspective = false;
    projectionMatrixDirty = true;
}

void Camera::setOrthographicByHeight(float height, float nearPlane, float farPlane) {
    this->orthoHeight = height;
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
        // Use the Vulkan version that flips Y
        // If your scene appears upside down, use Matrix4::perspective instead
        projectionMatrix = Matrix4::perspectiveVulkan(fovY, aspect, nearPlane, farPlane);
    } else {
        // Use stored ortho parameters or calculate from height
        if (ortho.left == ortho.right) {
            // Use height-based orthographic
            float halfHeight = orthoHeight * 0.5f;
            float halfWidth = halfHeight * aspect;
            projectionMatrix = Matrix4::orthographic(
                -halfWidth, halfWidth,
                -halfHeight, halfHeight,
                nearPlane, farPlane
            );
        } else {
            // Use explicit ortho parameters
            projectionMatrix = Matrix4::orthographic(
                ortho.left, ortho.right,
                ortho.bottom, ortho.top,
                nearPlane, farPlane
            );
        }
    }
}

void Camera::debugPrintMatrices() const {
    Matrix4 view = getViewMatrix();
    Matrix4 proj = getProjectionMatrix();

    std::cout << "\n=== Camera Debug Information ===" << std::endl;
    std::cout << "Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "Target: (" << target.x << ", " << target.y << ", " << target.z << ")" << std::endl;
    std::cout << "Up: (" << up.x << ", " << up.y << ", " << up.z << ")" << std::endl;
    std::cout << "Rotation: (" << rotation.x << ", " << rotation.y << ", " << rotation.z << ")" << std::endl;
    std::cout << "FOV: " << fovY << ", Aspect: " << aspect << std::endl;
    std::cout << "Near: " << nearPlane << ", Far: " << farPlane << std::endl;
    std::cout << "Mode: " << (isPerspective ? "Perspective" : "Orthographic") << std::endl;

    std::cout << "\nView Matrix:" << std::endl;
    const float* v = view.data();
    for (int row = 0; row < 4; row++) {
        std::cout << "  ";
        for (int col = 0; col < 4; col++) {
            // Print in row-major order for readability
            std::cout << v[col * 4 + row] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "\nProjection Matrix:" << std::endl;
    const float* p = proj.data();
    for (int row = 0; row < 4; row++) {
        std::cout << "  ";
        for (int col = 0; col < 4; col++) {
            // Print in row-major order for readability
            std::cout << p[col * 4 + row] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "================================\n" << std::endl;
}

void Camera::testMatrixOperations() {
    std::cout << "\n=== Testing Matrix Operations ===" << std::endl;

    // Test 1: Identity matrix
    Matrix4 identity = Matrix4::identity();
    Vector3 testVec(1, 2, 3);
    Vector3 result = identity * testVec;
    std::cout << "Identity * (1,2,3) = (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    std::cout << "  Expected: (1, 2, 3)" << std::endl;

    // Test 2: Translation
    Matrix4 trans = Matrix4::translation(Vector3(10, 20, 30));
    result = trans * testVec;
    std::cout << "Translate(10,20,30) * (1,2,3) = (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    std::cout << "  Expected: (11, 22, 33)" << std::endl;

    // Test 3: Scale
    Matrix4 scale = Matrix4::scale(Vector3(2, 3, 4));
    result = scale * testVec;
    std::cout << "Scale(2,3,4) * (1,2,3) = (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    std::cout << "  Expected: (2, 6, 12)" << std::endl;

    // Test 4: Combined transformation (scale then translate)
    Matrix4 combined = trans * scale;
    result = combined * testVec;
    std::cout << "Translate * Scale * (1,2,3) = (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    std::cout << "  Expected: (12, 26, 42)" << std::endl;

    // Test 5: Perspective projection of a point
    Matrix4 proj = Matrix4::perspective(45.0f, 1.0f, 0.1f, 100.0f);
    Vector3 point3d(0, 0, -5);
    result = proj * point3d;
    std::cout << "Perspective * (0,0,-5) = (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    std::cout << "  Expected: (0, 0, ~4.09)" << std::endl;

    // Test 6: View matrix
    Camera testCam;
    testCam.setPosition(Vector3(0, 0, 5));
    testCam.lookAt(Vector3(0, 0, 0));
    Matrix4 viewMat = testCam.getViewMatrix();
    Vector3 origin(0, 0, 0);
    result = viewMat * origin;
    std::cout << "View * origin = (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
    std::cout << "  Expected: (0, 0, -5) [origin in camera space]" << std::endl;

    std::cout << "==================================\n" << std::endl;
}
