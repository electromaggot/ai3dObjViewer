#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

class VulkanEngine;
class VulkanPipeline;
class Camera;
class Model;
class Light;

struct UniformBufferObject {
    alignas(16) float model[16];
    alignas(16) float view[16];
    alignas(16) float proj[16];
    alignas(16) float lightPos[4];
    alignas(16) float lightColor[4];
    alignas(16) float viewPos[4];
};

class Renderer {
public:
    Renderer(VulkanEngine& engine);
    ~Renderer();
    
    void render();
    
    // Scene management
    void setCamera(Camera* camera);
    void addModel(Model* model);
    void removeModel(Model* model);
    void clearModels();
    
    void setLight(Light* light);
    
private:
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    
    void updateUniformBuffer(uint32_t currentFrame);
    void recordCommandBuffer(VkCommandBuffer commandBuffer);
    
    VulkanEngine& engine;
    std::unique_ptr<VulkanPipeline> pipeline;
    
    Camera* camera;
    std::vector<Model*> models;
    Light* light;
    
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    
	uint32_t currentFrame;
    static const int MAX_FRAMES_IN_FLIGHT = 2;
};
