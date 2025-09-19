#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <unordered_map>

class VulkanEngine;
class VulkanPipeline;
class Camera;
class Model;
class Light;
class DynamicUBO;

// Global uniform data that's the same for all objects
struct GlobalUniformData {
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
    void createTextureDescriptorSetLayout();
    void createGraphicsPipeline();
    void createGlobalUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    void updateGlobalUniformBuffer(uint32_t currentFrame);
    void updateDynamicUBO(uint32_t currentFrame);
    void recordCommandBuffer(VkCommandBuffer commandBuffer);
    
    VulkanEngine& engine;
    std::unique_ptr<VulkanPipeline> pipeline;
    
    Camera* camera;
    std::vector<Model*> models;
    Light* light;
    
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout textureDescriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    // Texture descriptor sets - one per model with texture
    std::unordered_map<Model*, VkDescriptorSet> textureDescriptorSets;

    // Global uniform buffers (view, proj, lighting)
    std::vector<VkBuffer> globalUniformBuffers;
    std::vector<VkDeviceMemory> globalUniformBuffersMemory;
    std::vector<void*> globalUniformBuffersMapped;

    // Dynamic UBO for per-object transforms
    std::unique_ptr<DynamicUBO> dynamicUBO;

	uint32_t currentFrame;
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    static const uint32_t MAX_OBJECTS = 1000;
};
