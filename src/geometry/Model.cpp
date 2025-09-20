#include "Model.h"
#include "../utils/logger/Logging.h"
#include "math/Matrix4.h"
#include "rendering/Mesh.h"
#include "rendering/Texture.h"
#include "../utils/logger/Logging.h"

// Constructor should initialize transforms
Model::Model()
	: position(0.0f, 0.0f, 0.0f)
	, rotation(0.0f, 0.0f, 0.0f)
	, scale(1.0f, 1.0f, 1.0f)  // Default scale is 1,1,1 not 0,0,0!
	, visible(true)
{
}

Model::~Model() {
}

// CRITICAL: This function builds the model transformation matrix
Matrix4 Model::getModelMatrix() const {
	// Build transformation matrix in the correct order:
	// Scale first, then rotate, then translate
	// This is because we want to scale/rotate around the object's center,
	// then move it to its world position

	Matrix4 scaleMatrix = Matrix4::scale(scale);
	Matrix4 rotationMatrix = Matrix4::rotation(rotation);  // Euler angles in degrees
	Matrix4 translationMatrix = Matrix4::translation(position);

	// Combine: T * R * S
	// This means: scale first, then rotate, then translate
	Matrix4 modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	// Debug: Print some info about this matrix
	static int debugCounter = 3;
	if (debugCounter > 0 && position.length() > 0.01f) {
		debugCounter--;
		Log(LOW, "Model at position (%.2f, %.2f, %.2f)", position.x, position.y, position.z);
		Log(LOW, "  Scale: (%.2f, %.2f, %.2f)", scale.x, scale.y, scale.z);
		Log(LOW, "  Rotation: (%.2f, %.2f, %.2f)", rotation.x, rotation.y, rotation.z);

		// Check if the translation is in the matrix
		const float* data = modelMatrix.data();
		float tx = data[3 * 4 + 0];  // Column 3, row 0
		float ty = data[3 * 4 + 1];  // Column 3, row 1
		float tz = data[3 * 4 + 2];  // Column 3, row 2
		Log(LOW, "  Matrix translation: (%.2f, %.2f, %.2f)", tx, ty, tz);
	}

	return modelMatrix;
}

void Model::setPosition(const Vector3& pos) {
	position = pos;
}

void Model::setRotation(const Vector3& rot) {
	rotation = rot;
}

void Model::setScale(const Vector3& s) {
	scale = s;
}

void Model::setMesh(std::shared_ptr<Mesh> mesh) {
	this->mesh = mesh;
	buffersCreated = false;
}

void Model::setTexture(std::shared_ptr<Texture> texture) {
	this->texture = texture;
}

void Model::createBuffers(VulkanDevice& device) {
	if (mesh && !buffersCreated) {
		mesh->createBuffers(device);
		buffersCreated = true;
	}
}

void Model::render(VkCommandBuffer commandBuffer) {
	if (mesh && visible) {
		auto vertices = mesh->getVertices();
		auto indices = mesh->getIndices();

		static int debugCounter = 0;
		if (debugCounter < 10) {  // Only print first few times to avoid spam
			Log(LOW, "Model render: %zu vertices, %zu indices", vertices.size(), indices.size());
			if (!vertices.empty()) {
				Log(LOW, "First vertex: pos(%.2f, %.2f, %.2f) color(%.2f, %.2f, %.2f)",
					vertices[0].position.x, vertices[0].position.y, vertices[0].position.z,
					vertices[0].color.x, vertices[0].color.y, vertices[0].color.z);
			}
			debugCounter++;
		}

		mesh->bind(commandBuffer);
		mesh->draw(commandBuffer);
	}
}

void Model::updateModelMatrix() const {
	Matrix4 translation = Matrix4::translation(position);
	Matrix4 rotationMatrix = Matrix4::rotation(rotation);
	Matrix4 scaleMatrix = Matrix4::scale(scale);

	modelMatrix = translation * rotationMatrix * scaleMatrix;
}
