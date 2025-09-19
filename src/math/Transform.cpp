#include "Transform.h"

Transform::Transform()
	: position(0.0f, 0.0f, 0.0f)
	, rotation(0.0f, 0.0f, 0.0f)
	, scale(1.0f, 1.0f, 1.0f)
	, matrixDirty(true)
{
}

Transform::~Transform() {
}

void Transform::setPosition(const Vector3& position) {
	this->position = position;
	matrixDirty = true;
}

void Transform::translate(const Vector3& translation) {
	position += translation;
	matrixDirty = true;
}

void Transform::setRotation(const Vector3& rotation) {
	this->rotation = rotation;
	matrixDirty = true;
}

void Transform::rotate(const Vector3& rotation) {
	this->rotation += rotation;
	matrixDirty = true;
}

void Transform::setScale(const Vector3& scale) {
	this->scale = scale;
	matrixDirty = true;
}

void Transform::setScale(float uniformScale) {
	this->scale = Vector3(uniformScale, uniformScale, uniformScale);
	matrixDirty = true;
}

Matrix4 Transform::getMatrix() const {
	if (matrixDirty) {
		updateMatrix();
		matrixDirty = false;
	}
	return matrix;
}

Vector3 Transform::transformPoint(const Vector3& point) const {
	return getMatrix() * point;
}

Vector3 Transform::transformDirection(const Vector3& direction) const {
	// For directions, we don't want translation
	Matrix4 rotScale = Matrix4::rotation(rotation) * Matrix4::scale(scale);
	return rotScale * direction;
}

Vector3 Transform::transformNormal(const Vector3& normal) const {
	// For normals, we need the inverse transpose, but simplified here
	Matrix4 rot = Matrix4::rotation(rotation);
	return rot * normal;
}

void Transform::reset() {
	position = Vector3::zero();
	rotation = Vector3::zero();
	scale = Vector3::one();
	matrixDirty = true;
}

void Transform::updateMatrix() const {
	Matrix4 translation = Matrix4::translation(position);
	Matrix4 rotationMatrix = Matrix4::rotation(rotation);
	Matrix4 scaleMatrix = Matrix4::scale(scale);

	matrix = translation * rotationMatrix * scaleMatrix;
}