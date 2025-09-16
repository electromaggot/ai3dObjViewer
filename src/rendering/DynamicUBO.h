#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

class VulkanDevice;
class Matrix4;

// DynamicUBO manages a single large uniform buffer that contains transforms for multiple objects
// Each object's data is aligned according to GPU requirements for dynamic offsets
class DynamicUBO {
public:
    // Structure matching the shader's per-object uniform block
    struct PerObjectData {
        alignas(16) float model[16];      // Model matrix
        alignas(16) float normalMatrix[16]; // Normal matrix (inverse transpose of model)
    };

    DynamicUBO(VulkanDevice* device, uint32_t maxObjects, uint32_t framesInFlight);
    ~DynamicUBO();

    // Update the transform for a specific object in a specific frame
    void updateObjectTransform(uint32_t frameIndex, uint32_t objectIndex, const Matrix4& modelMatrix);

    // Get the dynamic offset for a specific object
    uint32_t getDynamicOffset(uint32_t objectIndex) const;

    // Get the aligned size of each object's data
    uint32_t getAlignedObjectSize() const { return alignedObjectSize; }

    // Get the buffer for a specific frame
    VkBuffer getBuffer(uint32_t frameIndex) const { return buffers[frameIndex]; }

    // Get descriptor buffer info for binding
    VkDescriptorBufferInfo getDescriptorBufferInfo(uint32_t frameIndex) const;

private:
    void createBuffers();
    void cleanupBuffers();

    VulkanDevice* device;
    uint32_t maxObjects;
    uint32_t framesInFlight;
    uint32_t alignedObjectSize;
    size_t totalBufferSize;

    // Per-frame buffers for double/triple buffering
    std::vector<VkBuffer> buffers;
    std::vector<VkDeviceMemory> memories;
    std::vector<void*> mappedMemory;
};