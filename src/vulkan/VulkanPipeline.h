#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanDevice;
class VulkanSwapchain;

class VulkanPipeline {
public:
    VulkanPipeline(VulkanDevice& device, VulkanSwapchain& swapchain, VkDescriptorSetLayout descriptorSetLayout);
    ~VulkanPipeline();
    
    VkPipeline getPipeline() const { return graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    
private:
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);
    
    VulkanDevice& device;
    VulkanSwapchain& swapchain;
    VkDescriptorSetLayout descriptorSetLayout;
    
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
};
