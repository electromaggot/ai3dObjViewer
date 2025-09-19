#pragma once

#include <vulkan/vulkan.h>
#include <string>

class VulkanUtils {
public:
	static std::string getVulkanResultString(VkResult result);
	static void checkResult(VkResult result, const std::string& operation);

	static bool hasStencilComponent(VkFormat format);
	static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	VulkanUtils() = delete;
};