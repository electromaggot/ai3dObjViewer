#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include <stdexcept>
#include <cstring>

VulkanBuffer::VulkanBuffer(VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	: device(device), buffer(VK_NULL_HANDLE), memory(VK_NULL_HANDLE), size(size), mappedData(nullptr) {

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device.getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.getLogicalDevice(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = device.findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device.getLogicalDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory");
	}

	vkBindBufferMemory(device.getLogicalDevice(), buffer, memory, 0);
}

VulkanBuffer::~VulkanBuffer() {
	if (mappedData) {
		unmap();
	}
	if (buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device.getLogicalDevice(), buffer, nullptr);
	}
	if (memory != VK_NULL_HANDLE) {
		vkFreeMemory(device.getLogicalDevice(), memory, nullptr);
	}
}

void VulkanBuffer::map() {
	vkMapMemory(device.getLogicalDevice(), memory, 0, size, 0, &mappedData);
}

void VulkanBuffer::unmap() {
	if (mappedData) {
		vkUnmapMemory(device.getLogicalDevice(), memory);
		mappedData = nullptr;
	}
}

void VulkanBuffer::copyTo(const void* data, VkDeviceSize size, VkDeviceSize offset) {
	if (!mappedData) {
		map();
	}
	memcpy(static_cast<char*>(mappedData) + offset, data, size);
}
