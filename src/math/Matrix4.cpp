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
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += m[k][j] * other.m[i][k];
            }
        }
    }
    
    return result;
}

Vector3 Matrix4::operator*(const Vector3& vec) const {
    float w = m[3][0] * vec.x + m[3][1] * vec.y + m[3][2] * vec.z + m[3][3];
    if (w != 0.0f) {
        return Vector3(
            (m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3]) / w,
            (m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3]) / w,
            (m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3]) / w
        );
    }
    return Vector3(
        m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3],
        m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3],
        m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3]
    );
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
    result.m[0][3] = translation.x;
    result.m[1][3] = translation.y;
    result.m[2][3] = translation.z;
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
    
    float tanHalfFovy = std::tan(fovY * PI / 360.0f);
    
    result.m[0][0] = 1.0f / (aspect * tanHalfFovy);
    result.m[1][1] = -1.0f / tanHalfFovy;  // NEGATIVE for Vulkan Y-flip
    result.m[2][2] = farPlane / (nearPlane - farPlane);
    result.m[2][3] = -(farPlane * nearPlane) / (farPlane - nearPlane);
    result.m[3][2] = -1.0f;
    
    return result;
}
/*Matrix4 Matrix4::perspective(float fovY, float aspect, float nearPlane, float farPlane) {
    Matrix4 result;
    memset(result.m.data(), 0, sizeof(result.m));
    
    float tanHalfFovy = std::tan(fovY * PI / 360.0f);
    
    result.m[0][0] = 1.0f / (aspect * tanHalfFovy);
    result.m[1][1] = 1.0f / tanHalfFovy;
    result.m[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.m[2][3] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    result.m[3][2] = -1.0f;
    
    return result;
}*/

Matrix4 Matrix4::lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 forward = (target - eye).normalized();
    Vector3 right = forward.cross(up).normalized();
    Vector3 newUp = right.cross(forward);
    
    Matrix4 result = identity();
    
    result.m[0][0] = right.x;
    result.m[0][1] = newUp.x;
    result.m[0][2] = -forward.x;
    result.m[0][3] = -right.dot(eye);
    
    result.m[1][0] = right.y;
    result.m[1][1] = newUp.y;
    result.m[1][2] = -forward.y;
    result.m[1][3] = -newUp.dot(eye);
    
    result.m[2][0] = right.z;
    result.m[2][1] = newUp.z;
    result.m[2][2] = -forward.z;
    result.m[2][3] = forward.dot(eye);
    
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

Vector3 Matrix4::getTranslation() const {
    return Vector3(m[0][3], m[1][3], m[2][3]);
}

Vector3 Matrix4::getScale() const {
    Vector3 scaleX(m[0][0], m[1][0], m[2][0]);
    Vector3 scaleY(m[0][1], m[1][1], m[2][1]);
    Vector3 scaleZ(m[0][2], m[1][2], m[2][2]);
    
    return Vector3(scaleX.length(), scaleY.length(), scaleZ.length());
}

void Matrix4::setTranslation(const Vector3& translation) {
    m[0][3] = translation.x;
    m[1][3] = translation.y;
    m[2][3] = translation.z;
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
    if (currentScale.x != 0.0f) {
        m[0][0] *= scale.x / currentScale.x;
        m[1][0] *= scale.x / currentScale.x;
        m[2][0] *= scale.x / currentScale.x;
    }
    if (currentScale.y != 0.0f) {
        m[0][1] *= scale.y / currentScale.y;
        m[1][1] *= scale.y / currentScale.y;
        m[2][1] *= scale.y / currentScale.y;
    }
    if (currentScale.z != 0.0f) {
        m[0][2] *= scale.z / currentScale.z;
        m[1][2] *= scale.z / currentScale.z;
        m[2][2] *= scale.z / currentScale.z;
    }
}

Matrix4 Matrix4::rotationX(float angle) {
    Matrix4 result = identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[1][1] = c;
    result.m[1][2] = -s;
    result.m[2][1] = s;
    result.m[2][2] = c;
    
    return result;
}

Matrix4 Matrix4::rotationY(float angle) {
    Matrix4 result = identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;
    
    return result;
}

Matrix4 Matrix4::rotationZ(float angle) {
    Matrix4 result = identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[0][0] = c;
    result.m[0][1] = -s;
    result.m[1][0] = s;
    result.m[1][1] = c;
    
    return result;
}
