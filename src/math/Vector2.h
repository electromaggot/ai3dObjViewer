#pragma once

#include <cmath>

class Vector2 {
public:
    float x, y;

    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x, float y) : x(x), y(y) {}

    // Basic operations
    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2 operator*(float scalar) const;
    Vector2 operator/(float scalar) const;

    Vector2& operator+=(const Vector2& other);
    Vector2& operator-=(const Vector2& other);
    Vector2& operator*=(float scalar);
    Vector2& operator/=(float scalar);

    bool operator==(const Vector2& other) const;
    bool operator!=(const Vector2& other) const;

    // Vector operations
    float dot(const Vector2& other) const;
    float length() const;
    float lengthSquared() const;
    Vector2 normalized() const;
    void normalize();

    // Static utility functions
    static Vector2 zero() { return Vector2(0.0f, 0.0f); }
    static Vector2 one() { return Vector2(1.0f, 1.0f); }

    static float distance(const Vector2& a, const Vector2& b);
    static Vector2 lerp(const Vector2& a, const Vector2& b, float t);
};