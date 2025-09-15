#include "Model.h"
#include "math/Matrix4.h"
#include "rendering/Mesh.h"
#include <iostream>

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
        std::cout << "Model at position (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        std::cout << "  Scale: (" << scale.x << ", " << scale.y << ", " << scale.z << ")" << std::endl;
        std::cout << "  Rotation: (" << rotation.x << ", " << rotation.y << ", " << rotation.z << ")" << std::endl;

        // Check if the translation is in the matrix
        const float* data = modelMatrix.data();
        float tx = data[3 * 4 + 0];  // Column 3, row 0
        float ty = data[3 * 4 + 1];  // Column 3, row 1
        float tz = data[3 * 4 + 2];  // Column 3, row 2
        std::cout << "  Matrix translation: (" << tx << ", " << ty << ", " << tz << ")" << std::endl;
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
            std::cout << "Model render: " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;
            if (!vertices.empty()) {
                std::cout << "First vertex: pos(" << vertices[0].position.x << ", " << vertices[0].position.y << ", " << vertices[0].position.z 
                         << ") color(" << vertices[0].color.x << ", " << vertices[0].color.y << ", " << vertices[0].color.z << ")" << std::endl;
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
