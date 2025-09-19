#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanEngine;

class VulkanDevice {
public:
    VulkanDevice(VkInstance instance, VkSurfaceKHR surface);
    ~VulkanDevice();
    
    VkDevice getLogicalDevice() const { return logicalDevice; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    
    uint32_t getGraphicsQueueFamily() const { return queueFamilies.graphicsFamily.value(); }
    uint32_t getPresentQueueFamily() const { return queueFamilies.presentFamily.value(); }
    
    QueueFamilyIndices getQueueFamilies() const { return queueFamilies; }
    SwapchainSupportDetails getSwapchainSupport() const;
    
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, 
                                VkImageTiling tiling, 
                                VkFormatFeatureFlags features) const;
    
private:
    void pickPhysicalDevice();
    void createLogicalDevice();
    
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) const;
    
    VkInstance instance;
    VkSurfaceKHR surface;
    
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    QueueFamilyIndices queueFamilies;
    
    static const std::vector<const char*> deviceExtensions;
};