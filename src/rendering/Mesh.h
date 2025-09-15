#pragma once

#include "../math/Vector3.h"
#include <vector>
#include <vulkan/vulkan.h>

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector3 color;
    
    static VkVertexInputBindingDescription getBindingDescription();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    
    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && color == other.color;
    }
};

class VulkanDevice;

class Mesh {
public:
    Mesh();
    ~Mesh();
    
    void setVertices(const std::vector<Vertex>& vertices);
    void setIndices(const std::vector<uint32_t>& indices);
    
    void createBuffers(VulkanDevice& device);
    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);
    
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }
    
    bool hasIndices() const { return !indices.empty(); }
    
private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    
    VulkanDevice* device;
    bool buffersCreated;
    
    void createVertexBuffer();
    void createIndexBuffer();
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};