#pragma once

#include <cmath>

class Vector3 {
public:
    float x, y, z;
    
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    // Basic operations
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;
    
    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3& operator*=(float scalar);
    Vector3& operator/=(float scalar);
    
    bool operator==(const Vector3& other) const;
    bool operator!=(const Vector3& other) const;
    
    // Vector operations
    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;
    float length() const;
    float lengthSquared() const;
    Vector3 normalized() const;
    void normalize();
    
    // Static utility functions
    static Vector3 zero() { return Vector3(0.0f, 0.0f, 0.0f); }
    static Vector3 one() { return Vector3(1.0f, 1.0f, 1.0f); }
    static Vector3 up() { return Vector3(0.0f, 1.0f, 0.0f); }
    static Vector3 right() { return Vector3(1.0f, 0.0f, 0.0f); }
    static Vector3 forward() { return Vector3(0.0f, 0.0f, -1.0f); }
    
    static float distance(const Vector3& a, const Vector3& b);
    static Vector3 lerp(const Vector3& a, const Vector3& b, float t);
};