#include "DynamicUBO.h"
#include "vulkan/VulkanDevice.h"
#include "math/Matrix4.h"
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <iostream>

DynamicUBO::DynamicUBO(VulkanDevice* device, uint32_t maxObjects, uint32_t framesInFlight)
    : device(device)
    , maxObjects(maxObjects)
    , framesInFlight(framesInFlight)
    , alignedObjectSize(0)
    , totalBufferSize(0) {

    // Get the minimum uniform buffer offset alignment requirement
    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &deviceProps);
    size_t minAlignment = deviceProps.limits.minUniformBufferOffsetAlignment;

    // Calculate aligned size for each object's data
    size_t objectSize = sizeof(PerObjectData);

    // Align to GPU requirements
    alignedObjectSize = static_cast<uint32_t>((objectSize + minAlignment - 1) & ~(minAlignment - 1));

    // Total buffer size for all objects
    totalBufferSize = alignedObjectSize * maxObjects;

    std::cout << "DynamicUBO: Creating buffers for " << maxObjects << " objects\n";
    std::cout << "  Object size: " << objectSize << " bytes\n";
    std::cout << "  Aligned size: " << alignedObjectSize << " bytes\n";
    std::cout << "  Min alignment: " << minAlignment << " bytes\n";
    std::cout << "  Total buffer size per frame: " << totalBufferSize << " bytes\n";

    createBuffers();
}

DynamicUBO::~DynamicUBO() {
    cleanupBuffers();
}

void DynamicUBO::createBuffers() {
    buffers.resize(framesInFlight);
    memories.resize(framesInFlight);
    mappedMemory.resize(framesInFlight);

    for (uint32_t i = 0; i < framesInFlight; ++i) {
        // Create buffer
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = totalBufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device->getLogicalDevice(), &bufferInfo, nullptr, &buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create dynamic uniform buffer!");
        }

        // Get memory requirements
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device->getLogicalDevice(), buffers[i], &memRequirements);

        // Allocate memory
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &memories[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate dynamic uniform buffer memory!");
        }

        // Bind buffer to memory
        vkBindBufferMemory(device->getLogicalDevice(), buffers[i], memories[i], 0);

        // Map memory for persistent mapping
        vkMapMemory(device->getLogicalDevice(), memories[i], 0, totalBufferSize, 0, &mappedMemory[i]);

        // Initialize memory to zero
        memset(mappedMemory[i], 0, totalBufferSize);
    }
}

void DynamicUBO::cleanupBuffers() {
    for (uint32_t i = 0; i < framesInFlight; ++i) {
        if (mappedMemory[i]) {
            vkUnmapMemory(device->getLogicalDevice(), memories[i]);
        }
        if (buffers[i]) {
            vkDestroyBuffer(device->getLogicalDevice(), buffers[i], nullptr);
        }
        if (memories[i]) {
            vkFreeMemory(device->getLogicalDevice(), memories[i], nullptr);
        }
    }
    buffers.clear();
    memories.clear();
    mappedMemory.clear();
}

void DynamicUBO::updateObjectTransform(uint32_t frameIndex, uint32_t objectIndex, const Matrix4& modelMatrix) {
    if (frameIndex >= framesInFlight) {
        throw std::runtime_error("Frame index out of bounds");
    }
    if (objectIndex >= maxObjects) {
        throw std::runtime_error("Object index out of bounds");
    }

    // Calculate offset for this object
    size_t offset = objectIndex * alignedObjectSize;

    // Get pointer to this object's data in the buffer
    uint8_t* bufferData = static_cast<uint8_t*>(mappedMemory[frameIndex]);
    PerObjectData* objectData = reinterpret_cast<PerObjectData*>(bufferData + offset);

    // Copy model matrix
    memcpy(objectData->model, modelMatrix.data(), sizeof(objectData->model));

    // For normal matrix, we'll use the model matrix for now
    // In a proper implementation, this should be inverse transpose of the upper-left 3x3
    // But for basic rendering, the model matrix works if there's no non-uniform scaling
    memcpy(objectData->normalMatrix, modelMatrix.data(), sizeof(objectData->normalMatrix));
}

uint32_t DynamicUBO::getDynamicOffset(uint32_t objectIndex) const {
    return objectIndex * alignedObjectSize;
}

VkDescriptorBufferInfo DynamicUBO::getDescriptorBufferInfo(uint32_t frameIndex) const {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffers[frameIndex];
    bufferInfo.offset = 0;
    bufferInfo.range = alignedObjectSize;  // Size of one object's data
    return bufferInfo;
}