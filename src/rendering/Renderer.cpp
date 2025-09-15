#include "Renderer.h"
#include "vulkan/VulkanEngine.h"
#include "vulkan/VulkanPipeline.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanSwapchain.h"
#include "Camera.h"
#include "Light.h"
#include "geometry/Model.h"
#include "math/Matrix4.h"
#include <stdexcept>
#include <cstring>
#include <array>
#include <iostream>

Renderer::Renderer(VulkanEngine& engine)
    : engine(engine)
    , camera(nullptr)
    , light(nullptr)
    , descriptorSetLayout(VK_NULL_HANDLE)
    , descriptorPool(VK_NULL_HANDLE)
    , currentFrame(0)
{
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

Renderer::~Renderer() {
    VulkanDevice* device = engine.getDevice();
    
    // Clean up uniform buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (uniformBuffers[i] != VK_NULL_HANDLE) {
            vkDestroyBuffer(device->getLogicalDevice(), uniformBuffers[i], nullptr);
            vkFreeMemory(device->getLogicalDevice(), uniformBuffersMemory[i], nullptr);
        }
    }
    
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device->getLogicalDevice(), descriptorPool, nullptr);
    }
    
    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device->getLogicalDevice(), descriptorSetLayout, nullptr);
    }
}

void Renderer::render() {
    VkCommandBuffer commandBuffer = engine.beginFrame();
    if (commandBuffer == nullptr) {
        return; // Skip frame if swapchain recreation needed
    }
    
    updateUniformBuffer(currentFrame);
    recordCommandBuffer(commandBuffer);
    
    engine.endFrame(commandBuffer);
    
    // Update frame counter
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::setCamera(Camera* camera) {
    this->camera = camera;
}

void Renderer::addModel(Model* model) {
    models.push_back(model);
    
    // Create buffers for the model
    if (model) {
        model->createBuffers(*engine.getDevice());
    }
}

void Renderer::removeModel(Model* model) {
    auto it = std::find(models.begin(), models.end(), model);
    if (it != models.end()) {
        models.erase(it);
    }
}

void Renderer::clearModels() {
    models.clear();
}

void Renderer::setLight(Light* light) {
    this->light = light;
}

void Renderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    
    if (vkCreateDescriptorSetLayout(engine.getDevice()->getLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void Renderer::createGraphicsPipeline() {
    pipeline = std::make_unique<VulkanPipeline>(*engine.getDevice(), *engine.getSwapchain(), descriptorSetLayout);
}

void Renderer::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(engine.getDevice()->getLogicalDevice(), &bufferInfo, nullptr, &uniformBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create uniform buffer");
        }
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(engine.getDevice()->getLogicalDevice(), uniformBuffers[i], &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = engine.getDevice()->findMemoryType(memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        if (vkAllocateMemory(engine.getDevice()->getLogicalDevice(), &allocInfo, nullptr, &uniformBuffersMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate uniform buffer memory");
        }
        
        vkBindBufferMemory(engine.getDevice()->getLogicalDevice(), uniformBuffers[i], uniformBuffersMemory[i], 0);
        vkMapMemory(engine.getDevice()->getLogicalDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void Renderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    
    if (vkCreateDescriptorPool(engine.getDevice()->getLogicalDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void Renderer::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();
    
    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(engine.getDevice()->getLogicalDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets");
    }
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        
        vkUpdateDescriptorSets(engine.getDevice()->getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

void Renderer::updateUniformBuffer(uint32_t currentFrame) {
    UniformBufferObject ubo{};
    
    // Initialize all matrices to identity
    Matrix4 identity = Matrix4::identity();
    memcpy(ubo.model, identity.data(), sizeof(ubo.model));
    memcpy(ubo.view, identity.data(), sizeof(ubo.view));
    memcpy(ubo.proj, identity.data(), sizeof(ubo.proj));
    
    if (camera) {
        Matrix4 view = camera->getViewMatrix();
        Matrix4 proj = camera->getProjectionMatrix();
        
		// NO TRANSPOSE - back to what was working
        memcpy(ubo.view, view.data(), sizeof(ubo.view));
        memcpy(ubo.proj, proj.data(), sizeof(ubo.proj));
        
        Vector3 viewPos = camera->getPosition();
        ubo.viewPos[0] = viewPos.x;
        ubo.viewPos[1] = viewPos.y;
        ubo.viewPos[2] = viewPos.z;
        ubo.viewPos[3] = 1.0f;
        
        // Debug camera matrices (only print occasionally)
        static int matrixDebugCounter = 0;
        if (matrixDebugCounter < 3) {
            std::cout << "Camera position: (" << viewPos.x << ", " << viewPos.y << ", " << viewPos.z << ")" << std::endl;
            std::cout << "View matrix [0]: " << view.data()[0] << ", " << view.data()[1] << ", " << view.data()[2] << ", " << view.data()[3] << std::endl;
            std::cout << "Proj matrix [0]: " << proj.data()[0] << ", " << proj.data()[1] << ", " << proj.data()[2] << ", " << proj.data()[3] << std::endl;
            matrixDebugCounter++;
        }
    } else {
        // Default camera position
        ubo.viewPos[0] = 0.0f;
        ubo.viewPos[1] = 0.0f;
        ubo.viewPos[2] = 10.0f;
        ubo.viewPos[3] = 1.0f;
    }
    
    // Default light if none set
    if (light) {
        Vector3 lightPos = light->getPosition();
        Vector3 lightColor = light->getColor();
        
        ubo.lightPos[0] = lightPos.x;
        ubo.lightPos[1] = lightPos.y;
        ubo.lightPos[2] = lightPos.z;
        ubo.lightPos[3] = 1.0f;
        
        ubo.lightColor[0] = lightColor.x;
        ubo.lightColor[1] = lightColor.y;
        ubo.lightColor[2] = lightColor.z;
        ubo.lightColor[3] = 1.0f;
    } else {
        // Default light
        ubo.lightPos[0] = 2.0f;
        ubo.lightPos[1] = 2.0f;
        ubo.lightPos[2] = 2.0f;
        ubo.lightPos[3] = 1.0f;
        
        ubo.lightColor[0] = 1.0f;
        ubo.lightColor[1] = 1.0f;
        ubo.lightColor[2] = 1.0f;
        ubo.lightColor[3] = 1.0f;
    }
    
    memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer) {
    VulkanSwapchain* swapchain = engine.getSwapchain();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapchain->getRenderPass();
    renderPassInfo.framebuffer = swapchain->getFramebuffer(engine.getCurrentImageIndex());
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain->getExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.1f, 0.1f, 0.3f, 1.0f}};  // Dark blue background
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain->getExtent().width);
    viewport.height = static_cast<float>(swapchain->getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Bind the descriptor set once
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipeline->getPipelineLayout(), 0, 1,
                           &descriptorSets[currentFrame], 0, nullptr);

    // Debug: Print model count (only first few frames)
    static int debugCount = 3;
    if (debugCount > 0) {
        --debugCount;
        std::cout << "Rendering frame with " << models.size() << " models" << std::endl;
    }

    // FIXED: Update uniform buffer and draw each model separately
    for (size_t i = 0; i < models.size(); ++i) {
        Model* model = models[i];
        if (model && model->isVisible()) {
            // Get the current uniform buffer data
            UniformBufferObject ubo{};

            // Set model matrix for this specific model
            Matrix4 modelMatrix = model->getModelMatrix();
            memcpy(ubo.model, modelMatrix.data(), sizeof(ubo.model));

            // Copy view and projection matrices from camera
            if (camera) {
                Matrix4 view = camera->getViewMatrix();
                Matrix4 proj = camera->getProjectionMatrix();
                memcpy(ubo.view, view.data(), sizeof(ubo.view));
                memcpy(ubo.proj, proj.data(), sizeof(ubo.proj));

                Vector3 viewPos = camera->getPosition();
                ubo.viewPos[0] = viewPos.x;
                ubo.viewPos[1] = viewPos.y;
                ubo.viewPos[2] = viewPos.z;
                ubo.viewPos[3] = 1.0f;
            }

            // Set lighting
            if (light) {
                Vector3 lightPos = light->getPosition();
                Vector3 lightColor = light->getColor();

                ubo.lightPos[0] = lightPos.x;
                ubo.lightPos[1] = lightPos.y;
                ubo.lightPos[2] = lightPos.z;
                ubo.lightPos[3] = 1.0f;

                ubo.lightColor[0] = lightColor.x;
                ubo.lightColor[1] = lightColor.y;
                ubo.lightColor[2] = lightColor.z;
                ubo.lightColor[3] = 1.0f;
            } else {
                // Default light
                ubo.lightPos[0] = 2.0f;
                ubo.lightPos[1] = 2.0f;
                ubo.lightPos[2] = 2.0f;
                ubo.lightPos[3] = 1.0f;

                ubo.lightColor[0] = 1.0f;
                ubo.lightColor[1] = 1.0f;
                ubo.lightColor[2] = 1.0f;
                ubo.lightColor[3] = 1.0f;
            }

            // Update uniform buffer with this model's data
            memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

            // IMPORTANT: We need a pipeline barrier or flush to ensure the uniform buffer
            // update is visible to the GPU before drawing
            VkMemoryBarrier memoryBarrier{};
            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                VK_PIPELINE_STAGE_HOST_BIT,
                                VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                                0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

            // Now draw this model
            model->render(commandBuffer);

            if (debugCount > 0) {
                Vector3 pos = model->getPosition();
                std::cout << "  Drew model " << i << " at ("
                         << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
            }
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}
