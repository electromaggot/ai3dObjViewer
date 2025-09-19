#include "Vector3.h"

Vector3 Vector3::operator+(const Vector3& other) const {
	return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(const Vector3& other) const {
	return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator*(float scalar) const {
	return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator/(float scalar) const {
	return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3& Vector3::operator+=(const Vector3& other) {
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

Vector3& Vector3::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

Vector3& Vector3::operator/=(float scalar) {
	x /= scalar;
	y /= scalar;
	z /= scalar;
	return *this;
}

bool Vector3::operator==(const Vector3& other) const {
	const float epsilon = 1e-6f;
	return std::abs(x - other.x) < epsilon &&
		   std::abs(y - other.y) < epsilon &&
		   std::abs(z - other.z) < epsilon;
}

bool Vector3::operator!=(const Vector3& other) const {
	return !(*this == other);
}

float Vector3::dot(const Vector3& other) const {
	return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::cross(const Vector3& other) const {
	return Vector3(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

float Vector3::length() const {
	return std::sqrt(x * x + y * y + z * z);
}

float Vector3::lengthSquared() const {
	return x * x + y * y + z * z;
}

Vector3 Vector3::normalized() const {
	float len = length();
	if (len > 0.0f) {
		return *this / len;
	}
	return Vector3::zero();
}

void Vector3::normalize() {
	float len = length();
	if (len > 0.0f) {
		x /= len;
		y /= len;
		z /= len;
	}
}

float Vector3::distance(const Vector3& a, const Vector3& b) {
	return (b - a).length();
}

Vector3 Vector3::lerp(const Vector3& a, const Vector3& b, float t) {
	return a + (b - a) * t;
}