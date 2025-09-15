#pragma once

#include "../math/Vector3.h"
#include "../math/Matrix4.h"

struct Rect {
	float left, right, bottom, top;
	Rect( float l, float r, float b, float t) {
		left = l; right = r; bottom = b; top = t;
	}
};

class Camera {
public:
    Camera();
    virtual ~Camera() = default;

    // Transform operations
    void setPosition(const Vector3& position);
    void setTarget(const Vector3& target);
    void setUp(const Vector3& up);

    void move(const Vector3& movement);
    void rotate(const Vector3& rotation);
    void lookAt(const Vector3& target);

    // Projection settings
    void setPerspective(float fovY, float aspect, float nearPlane, float farPlane);
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void setOrthographicByHeight(float height, float nearPlane = 0.1f, float farPlane = 100.0f);

    // Getters
    Vector3 getPosition() const { return position; }
    Vector3 getTarget() const { return target; }
    Vector3 getUp() const { return up; }
    Vector3 getForward() const { return (target - position).normalized(); }
    Vector3 getRight() const { return getForward().cross(up).normalized(); }
    Vector3 getRotation() const { return rotation; }

    Matrix4 getViewMatrix() const;
    Matrix4 getProjectionMatrix() const;
    Matrix4 getViewProjectionMatrix() const;

    // Camera parameters
    float getFovY() const { return fovY; }
    float getAspect() const { return aspect; }
    float getNearPlane() const { return nearPlane; }
    float getFarPlane() const { return farPlane; }
    bool getIsPerspective() const { return isPerspective; }

    void setAspectRatio(float aspect);

    // Debug methods
    void debugPrintMatrices() const;
    static void testMatrixOperations();

protected:
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;

    Vector3 position;
    Vector3 target;
    Vector3 up;

    // Rotation state for smooth movement
    Vector3 rotation; // Euler angles in degrees

    // Projection parameters
    float fovY;
    float aspect;
    float nearPlane;
    float farPlane;

    // Orthographic parameters
	Rect  ortho;
    float orthoHeight;

    // Cached matrices
    mutable Matrix4 viewMatrix;
    mutable Matrix4 projectionMatrix;
    mutable bool viewMatrixDirty;
    mutable bool projectionMatrixDirty;

    bool isPerspective;
};
