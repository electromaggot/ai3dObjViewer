#include "Matrix4.h"
#include <cmath>
#include <cstring>

const float PI = 3.14159265359f;

Matrix4::Matrix4() {
	memset(m.data(), 0, sizeof(m));
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

Matrix4::Matrix4(const std::array<std::array<float, 4>, 4>& matrix) : m(matrix) {
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
	Matrix4 result;

	// FIXED: Correct multiplication for column-major matrices
	// m[col][row] layout, so m[i] is column i
	for (int col = 0; col < 4; ++col) {
		for (int row = 0; row < 4; ++row) {
			result.m[col][row] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				// result column = this * other column
				result.m[col][row] += m[k][row] * other.m[col][k];
			}
		}
	}

	return result;
}

Vector3 Matrix4::operator*(const Vector3& vec) const {
	// For column-major matrices: m[col][row]
	// Transform as a point (w=1)
	float x = m[0][0] * vec.x + m[1][0] * vec.y + m[2][0] * vec.z + m[3][0];
	float y = m[0][1] * vec.x + m[1][1] * vec.y + m[2][1] * vec.z + m[3][1];
	float z = m[0][2] * vec.x + m[1][2] * vec.y + m[2][2] * vec.z + m[3][2];
	float w = m[0][3] * vec.x + m[1][3] * vec.y + m[2][3] * vec.z + m[3][3];

	if (std::abs(w) > 0.00001f && std::abs(w - 1.0f) > 0.00001f) {
		return Vector3(x / w, y / w, z / w);
	}
	return Vector3(x, y, z);
}

Matrix4& Matrix4::operator*=(const Matrix4& other) {
	*this = *this * other;
	return *this;
}

Matrix4 Matrix4::identity() {
	return Matrix4();
}

Matrix4 Matrix4::translation(const Vector3& translation) {
	Matrix4 result = identity();
	// Translation goes in the last column for column-major
	result.m[3][0] = translation.x;
	result.m[3][1] = translation.y;
	result.m[3][2] = translation.z;
	return result;
}

Matrix4 Matrix4::rotation(const Vector3& rotation) {
	Matrix4 rotX = rotationX(rotation.x * PI / 180.0f);
	Matrix4 rotY = rotationY(rotation.y * PI / 180.0f);
	Matrix4 rotZ = rotationZ(rotation.z * PI / 180.0f);
	return rotZ * rotY * rotX;
}

Matrix4 Matrix4::scale(const Vector3& scale) {
	Matrix4 result = identity();
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	return result;
}

Matrix4 Matrix4::perspective(float fovY, float aspect, float nearPlane, float farPlane) {
	Matrix4 result;
	memset(result.m.data(), 0, sizeof(result.m));

	float tanHalfFovy = std::tan(fovY * PI / 360.0f);  // fovY is in degrees

	// Column 0: X scaling
	result.m[0][0] = 1.0f / (aspect * tanHalfFovy);

	// Column 1: Y scaling (positive for right-handed, no flip here)
	result.m[1][1] = 1.0f / tanHalfFovy;

	// Column 2: Z mapping for Vulkan [0, 1] depth range
	// For right-handed system looking down -Z
	result.m[2][2] = farPlane / (farPlane - nearPlane);
	result.m[2][3] = 1.0f;  // Set w = z for perspective divide

	// Column 3: Translation
	result.m[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);

	return result;
}

Matrix4 Matrix4::perspectiveVulkan(float fovY, float aspect, float nearPlane, float farPlane) {
	Matrix4 result;
	memset(result.m.data(), 0, sizeof(result.m));

	float tanHalfFovy = std::tan(fovY * PI / 360.0f);  // fovY is in degrees

	// Column 0: X scaling
	result.m[0][0] = 1.0f / (aspect * tanHalfFovy);

	// Column 1: Y scaling - NEGATIVE for Vulkan (Y points down in NDC)
	result.m[1][1] = -1.0f / tanHalfFovy;

	// Column 2: Z mapping for Vulkan [0, 1] depth range
	// Maps near to 0, far to 1
	result.m[2][2] = farPlane / (farPlane - nearPlane);
	result.m[2][3] = 1.0f;  // Set w = z for perspective divide

	// Column 3: Translation
	result.m[3][2] = -(farPlane * nearPlane) / (farPlane - nearPlane);

	return result;
}

Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
	Matrix4 result;
	memset(result.m.data(), 0, sizeof(result.m));

	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farPlane - nearPlane);  // Vulkan [0,1] depth

	result.m[3][0] = -(right + left) / (right - left);
	result.m[3][1] = -(top + bottom) / (top - bottom);
	result.m[3][2] = -nearPlane / (farPlane - nearPlane);
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4 Matrix4::lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
	// Standard lookAt implementation for right-handed coordinate system
	// In right-handed system: camera looks down -Z axis
	Vector3 zaxis = (eye - target).normalized();	// Camera's Z axis (backward)
	Vector3 xaxis = up.cross(zaxis).normalized();	// Camera's X axis (right)
	Vector3 yaxis = zaxis.cross(xaxis);				// Camera's Y axis (up)

	Matrix4 result = identity();

	// Column-major layout: m[col][row]
	// Rotation part (inverse of camera orientation)
	result.m[0][0] = xaxis.x;
	result.m[1][0] = xaxis.y;
	result.m[2][0] = xaxis.z;
	result.m[0][1] = yaxis.x;
	result.m[1][1] = yaxis.y;
	result.m[2][1] = yaxis.z;
	result.m[0][2] = zaxis.x;
	result.m[1][2] = zaxis.y;
	result.m[2][2] = zaxis.z;

	// Translation part (negative dot product with basis vectors)
	result.m[3][0] = -xaxis.dot(eye);
	result.m[3][1] = -yaxis.dot(eye);
	result.m[3][2] = -zaxis.dot(eye);

	return result;
}

Matrix4 Matrix4::transposed() const {
	Matrix4 result;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m[j][i];
		}
	}
	return result;
}

Matrix4 Matrix4::inverted() const {
	// Simplified inverse for now - you can implement full inverse if needed
	// This assumes the matrix is orthogonal (rotation + translation only)
	Matrix4 result = transposed();

	// Fix the translation part
	Vector3 translation = getTranslation();
	result.setTranslation(Vector3(
		-result.m[0][0] * translation.x - result.m[0][1] * translation.y - result.m[0][2] * translation.z,
		-result.m[1][0] * translation.x - result.m[1][1] * translation.y - result.m[1][2] * translation.z,
		-result.m[2][0] * translation.x - result.m[2][1] * translation.y - result.m[2][2] * translation.z
	));

	return result;
}

float Matrix4::determinant() const {
	// Implementation of 4x4 determinant
	float det = 0.0f;

	det += m[0][0] * (m[1][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3]) -
					  m[2][1] * (m[1][2] * m[3][3] - m[3][2] * m[1][3]) +
					  m[3][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3]));

	det -= m[1][0] * (m[0][1] * (m[2][2] * m[3][3] - m[3][2] * m[2][3]) -
					  m[2][1] * (m[0][2] * m[3][3] - m[3][2] * m[0][3]) +
					  m[3][1] * (m[0][2] * m[2][3] - m[2][2] * m[0][3]));

	det += m[2][0] * (m[0][1] * (m[1][2] * m[3][3] - m[3][2] * m[1][3]) -
					  m[1][1] * (m[0][2] * m[3][3] - m[3][2] * m[0][3]) +
					  m[3][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3]));

	det -= m[3][0] * (m[0][1] * (m[1][2] * m[2][3] - m[2][2] * m[1][3]) -
					  m[1][1] * (m[0][2] * m[2][3] - m[2][2] * m[0][3]) +
					  m[2][1] * (m[0][2] * m[1][3] - m[1][2] * m[0][3]));

	return det;
}

Vector3 Matrix4::getTranslation() const {
	// Translation is in the last column for column-major
	return Vector3(m[3][0], m[3][1], m[3][2]);
}

Vector3 Matrix4::getScale() const {
	// Scale is the length of each basis vector (first 3 columns)
	Vector3 scaleX(m[0][0], m[0][1], m[0][2]);
	Vector3 scaleY(m[1][0], m[1][1], m[1][2]);
	Vector3 scaleZ(m[2][0], m[2][1], m[2][2]);

	return Vector3(scaleX.length(), scaleY.length(), scaleZ.length());
}

void Matrix4::setTranslation(const Vector3& translation) {
	// Translation goes in the last column for column-major
	m[3][0] = translation.x;
	m[3][1] = translation.y;
	m[3][2] = translation.z;
}

void Matrix4::setRotation(const Vector3& rotation) {
	Matrix4 rot = Matrix4::rotation(rotation);
	Vector3 scale = getScale();
	Vector3 translation = getTranslation();

	// Preserve scale and translation, only change rotation
	*this = Matrix4::translation(translation) * rot * Matrix4::scale(scale);
}

void Matrix4::setScale(const Vector3& scale) {
	Vector3 currentScale = getScale();

	// Scale each basis vector (column)
	if (currentScale.x != 0.0f) {
		float factor = scale.x / currentScale.x;
		m[0][0] *= factor;
		m[0][1] *= factor;
		m[0][2] *= factor;
	}
	if (currentScale.y != 0.0f) {
		float factor = scale.y / currentScale.y;
		m[1][0] *= factor;
		m[1][1] *= factor;
		m[1][2] *= factor;
	}
	if (currentScale.z != 0.0f) {
		float factor = scale.z / currentScale.z;
		m[2][0] *= factor;
		m[2][1] *= factor;
		m[2][2] *= factor;
	}
}

Matrix4 Matrix4::rotationX(float angle) {
	Matrix4 result = identity();
	float c = std::cos(angle);
	float s = std::sin(angle);

	result.m[1][1] = c;
	result.m[2][1] = s;
	result.m[1][2] = -s;
	result.m[2][2] = c;

	return result;
}

Matrix4 Matrix4::rotationY(float angle) {
	Matrix4 result = identity();
	float c = std::cos(angle);
	float s = std::sin(angle);

	result.m[0][0] = c;
	result.m[2][0] = -s;
	result.m[0][2] = s;
	result.m[2][2] = c;

	return result;
}

Matrix4 Matrix4::rotationZ(float angle) {
	Matrix4 result = identity();
	float c = std::cos(angle);
	float s = std::sin(angle);

	result.m[0][0] = c;
	result.m[1][0] = s;
	result.m[0][1] = -s;
	result.m[1][1] = c;

	return result;
}
