#pragma once

#include "Vector3.h"
#include <array>

class Matrix4 {
public:
	// Column-major order (OpenGL/Vulkan style)
	std::array<std::array<float, 4>, 4> m;

	Matrix4();
	Matrix4(const std::array<std::array<float, 4>, 4>& matrix);

	// Element access
	float& operator()(int row, int col) { return m[col][row]; }
	const float& operator()(int row, int col) const { return m[col][row]; }

	// Matrix operations
	Matrix4 operator*(const Matrix4& other) const;
	Vector3 operator*(const Vector3& vec) const;
	Matrix4& operator*=(const Matrix4& other);

	// Transformation functions
	static Matrix4 identity();
	static Matrix4 translation(const Vector3& translation);
	static Matrix4 rotation(const Vector3& rotation); // Euler angles in degrees
	static Matrix4 scale(const Vector3& scale);
	static Matrix4 perspective(float fovY, float aspect, float nearPlane, float farPlane);
	static Matrix4 perspectiveVulkan(float fovY, float aspect, float nearPlane, float farPlane);
	static Matrix4 orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
	static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up);

	// Utility functions
	Matrix4 transposed() const;
	Matrix4 inverted() const;
	float determinant() const;

	// Get transformation components
	Vector3 getTranslation() const;
	Vector3 getScale() const;

	// Set transformation
	void setTranslation(const Vector3& translation);
	void setRotation(const Vector3& rotation);
	void setScale(const Vector3& scale);

	// Get raw data pointer (for sending to shaders)
	const float* data() const { return &m[0][0]; }
	float* data() { return &m[0][0]; }

private:
	static Matrix4 rotationX(float angle);
	static Matrix4 rotationY(float angle);
	static Matrix4 rotationZ(float angle);
};
