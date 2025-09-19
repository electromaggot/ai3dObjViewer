#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanDevice;
class VulkanSwapchain;

enum class PipelineType {
    UNTEXTURED,
    TEXTURED
};

class VulkanPipeline {
public:
    VulkanPipeline(VulkanDevice& device, VulkanSwapchain& swapchain, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout);
    ~VulkanPipeline();

    VkPipeline getPipeline(PipelineType type) const {
        return type == PipelineType::TEXTURED ? texturedPipeline : untexturedPipeline;
    }
    VkPipelineLayout getPipelineLayout(PipelineType type) const {
        return type == PipelineType::TEXTURED ? texturedPipelineLayout : untexturedPipelineLayout;
    }

private:
    void createGraphicsPipelines();
    void createUntexturedPipeline();
    void createTexturedPipeline();
    VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);

    VulkanDevice& device;
    VulkanSwapchain& swapchain;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout textureDescriptorSetLayout;

    // Untextured pipeline
    VkPipelineLayout untexturedPipelineLayout;
    VkPipeline untexturedPipeline;

    // Textured pipeline
    VkPipelineLayout texturedPipelineLayout;
    VkPipeline texturedPipeline;
};
