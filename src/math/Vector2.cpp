#include "Vector2.h"

Vector2 Vector2::operator+(const Vector2& other) const {
    return Vector2(x + other.x, y + other.y);
}

Vector2 Vector2::operator-(const Vector2& other) const {
    return Vector2(x - other.x, y - other.y);
}

Vector2 Vector2::operator*(float scalar) const {
    return Vector2(x * scalar, y * scalar);
}

Vector2 Vector2::operator/(float scalar) const {
    return Vector2(x / scalar, y / scalar);
}

Vector2& Vector2::operator+=(const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2& Vector2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2& Vector2::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

bool Vector2::operator==(const Vector2& other) const {
    const float epsilon = 0.00001f;
    return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
}

bool Vector2::operator!=(const Vector2& other) const {
    return !(*this == other);
}

float Vector2::dot(const Vector2& other) const {
    return x * other.x + y * other.y;
}

float Vector2::length() const {
    return std::sqrt(x * x + y * y);
}

float Vector2::lengthSquared() const {
    return x * x + y * y;
}

Vector2 Vector2::normalized() const {
    float len = length();
    if (len > 0.0f) {
        return *this / len;
    }
    return Vector2::zero();
}

void Vector2::normalize() {
    float len = length();
    if (len > 0.0f) {
        *this /= len;
    }
}

float Vector2::distance(const Vector2& a, const Vector2& b) {
    return (b - a).length();
}

Vector2 Vector2::lerp(const Vector2& a, const Vector2& b, float t) {
    return a + (b - a) * t;
}