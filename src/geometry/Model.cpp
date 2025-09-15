#include "Model.h"
#include "rendering/Mesh.h"
#include "vulkan/VulkanDevice.h"
#include <iostream>

Model::Model()
    : position(0.0f, 0.0f, 0.0f)
    , rotation(0.0f, 0.0f, 0.0f)
    , scale(1.0f, 1.0f, 1.0f)
    , visible(true)
    , buffersCreated(false)
    , modelMatrixDirty(true)
{
}

Model::~Model() {
}

void Model::setPosition(const Vector3& position) {
    this->position = position;
    modelMatrixDirty = true;
}

void Model::setRotation(const Vector3& rotation) {
    this->rotation = rotation;
    modelMatrixDirty = true;
}

void Model::setScale(const Vector3& scale) {
    this->scale = scale;
    modelMatrixDirty = true;
}

Matrix4 Model::getModelMatrix() const {
    if (modelMatrixDirty) {
        updateModelMatrix();
        modelMatrixDirty = false;
    }
    return modelMatrix;
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
