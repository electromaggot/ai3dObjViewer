#pragma once

#include <vulkan/vulkan.h>

class VulkanDevice;

class VulkanBuffer {
public:
    VulkanBuffer(VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~VulkanBuffer();
    
    VkBuffer getBuffer() const { return buffer; }
    VkDeviceMemory getMemory() const { return memory; }
    void* getMappedData() const { return mappedData; }
    
    void map();
    void unmap();
    void copyTo(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    
private:
    VulkanDevice& device;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
    void* mappedData;
};
