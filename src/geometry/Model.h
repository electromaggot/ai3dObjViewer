#pragma once

#include "math/Vector3.h"
#include "math/Matrix4.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <string>

class Mesh;
class VulkanDevice;
class Texture;

class Model {
public:
    Model();
    ~Model();
    
    // Transform operations
    void setPosition(const Vector3& position);
    void setRotation(const Vector3& rotation);
    void setScale(const Vector3& scale);
    
    Vector3 getPosition() const { return position; }
    Vector3 getRotation() const { return rotation; }
    Vector3 getScale() const { return scale; }
    
    // Model matrix
    Matrix4 getModelMatrix() const;
    
    // Mesh operations
    void setMesh(std::shared_ptr<Mesh> mesh);
    std::shared_ptr<Mesh> getMesh() const { return mesh; }

    // Texture operations
    void setTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture() const { return texture; }
    bool hasTexture() const { return texture != nullptr; }

    // Rendering
    void createBuffers(VulkanDevice& device);
    void render(VkCommandBuffer commandBuffer);
    
    // Utility
    bool isVisible() const { return visible; }
    void setVisible(bool visible) { this->visible = visible; }
    
private:
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Texture> texture;
    bool visible;
    bool buffersCreated;
    
    mutable Matrix4 modelMatrix;
    
    void updateModelMatrix() const;
};
