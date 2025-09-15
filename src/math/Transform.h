#pragma once

#include "Vector3.h"
#include "Matrix4.h"

class Transform {
public:
    Transform();
    ~Transform();
    
    // Position
    void setPosition(const Vector3& position);
    Vector3 getPosition() const { return position; }
    void translate(const Vector3& translation);
    
    // Rotation (Euler angles in degrees)
    void setRotation(const Vector3& rotation);
    Vector3 getRotation() const { return rotation; }
    void rotate(const Vector3& rotation);
    
    // Scale
    void setScale(const Vector3& scale);
    void setScale(float uniformScale);
    Vector3 getScale() const { return scale; }
    
    // Transform matrix
    Matrix4 getMatrix() const;
    
    // Transform operations
    Vector3 transformPoint(const Vector3& point) const;
    Vector3 transformDirection(const Vector3& direction) const;
    Vector3 transformNormal(const Vector3& normal) const;
    
    // Utility
    void reset();
    
private:
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    
    mutable Matrix4 matrix;
    mutable bool matrixDirty;
    
    void updateMatrix() const;
};