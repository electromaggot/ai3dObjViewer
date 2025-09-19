#include "Renderer.h"
#include "vulkan/VulkanEngine.h"
#include "vulkan/VulkanPipeline.h"
#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanSwapchain.h"
#include "Camera.h"
#include "Light.h"
#include "DynamicUBO.h"
#include "geometry/Model.h"
#include "Mesh.h"
#include "Texture.h"
#include "math/Matrix4.h"
#include <stdexcept>
#include <cstring>
#include <array>
#include <iostream>

// Define static constants
const int Renderer::MAX_FRAMES_IN_FLIGHT;
const uint32_t Renderer::MAX_OBJECTS;

Renderer::Renderer(VulkanEngine& engine)
    : engine(engine)
    , camera(nullptr)
    , light(nullptr)
    , descriptorSetLayout(VK_NULL_HANDLE)
    , textureDescriptorSetLayout(VK_NULL_HANDLE)
    , descriptorPool(VK_NULL_HANDLE)
    , currentFrame(0)
{
    createDescriptorSetLayout();
    createTextureDescriptorSetLayout();
    createGraphicsPipeline();

    // Create dynamic UBO for per-object transforms
    dynamicUBO = std::make_unique<DynamicUBO>(engine.getDevice(), MAX_OBJECTS, MAX_FRAMES_IN_FLIGHT);

    createGlobalUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
}

Renderer::~Renderer() {
    VulkanDevice* device = engine.getDevice();

    // Clean up global uniform buffers
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (globalUniformBuffers[i] != VK_NULL_HANDLE) {
            vkDestroyBuffer(device->getLogicalDevice(), globalUniformBuffers[i], nullptr);
            vkFreeMemory(device->getLogicalDevice(), globalUniformBuffersMemory[i], nullptr);
        }
    }
    
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device->getLogicalDevice(), descriptorPool, nullptr);
    }
    
    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device->getLogicalDevice(), descriptorSetLayout, nullptr);
    }
    if (textureDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device->getLogicalDevice(), textureDescriptorSetLayout, nullptr);
    }
}

void Renderer::render() {
    VkCommandBuffer commandBuffer = engine.beginFrame();
    if (commandBuffer == nullptr) {
        return; // Skip frame if swapchain recreation needed
    }

    updateGlobalUniformBuffer(currentFrame);
    updateDynamicUBO(currentFrame);
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

        // Create texture descriptor set if the model has a texture
        if (model->hasTexture() && model->getTexture()) {
            // Allocate descriptor set
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &textureDescriptorSetLayout;

            VkDescriptorSet textureDescriptorSet;
            if (vkAllocateDescriptorSets(engine.getDevice()->getLogicalDevice(), &allocInfo, &textureDescriptorSet) != VK_SUCCESS) {
                throw std::runtime_error("Failed to allocate texture descriptor set");
            }

            // Update the descriptor set with the texture
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = model->getTexture()->getImageView();
            imageInfo.sampler = model->getTexture()->getSampler();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = textureDescriptorSet;
            descriptorWrite.dstBinding = 0; // Binding 0 in set 1
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(engine.getDevice()->getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

            // Store the descriptor set
            textureDescriptorSets[model] = textureDescriptorSet;

            std::cout << "Created texture descriptor set for model at " << model << std::endl;
        }
    }
}

void Renderer::removeModel(Model* model) {
    auto it = std::find(models.begin(), models.end(), model);
    if (it != models.end()) {
        models.erase(it);
    }

    // Remove texture descriptor set if it exists
    auto textureIt = textureDescriptorSets.find(model);
    if (textureIt != textureDescriptorSets.end()) {
        textureDescriptorSets.erase(textureIt);
    }
}

void Renderer::clearModels() {
    models.clear();
    textureDescriptorSets.clear();
}

void Renderer::setLight(Light* light) {
    this->light = light;
}

void Renderer::createDescriptorSetLayout() {
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};

    // Binding 0: Global uniforms (view, proj, lighting)
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // Binding 1: Dynamic UBO for per-object transforms
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    if (vkCreateDescriptorSetLayout(engine.getDevice()->getLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }
}

void Renderer::createTextureDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;  // Binding 0 in set 1 (texture descriptor set)
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(engine.getDevice()->getLogicalDevice(), &layoutInfo, nullptr, &textureDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture descriptor set layout");
    }
}

void Renderer::createGraphicsPipeline() {
    pipeline = std::make_unique<VulkanPipeline>(*engine.getDevice(), *engine.getSwapchain(), descriptorSetLayout, textureDescriptorSetLayout);
}

void Renderer::createGlobalUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(GlobalUniformData);

    globalUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    globalUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    globalUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        
        if (vkCreateBuffer(engine.getDevice()->getLogicalDevice(), &bufferInfo, nullptr, &globalUniformBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create global uniform buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(engine.getDevice()->getLogicalDevice(), globalUniformBuffers[i], &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = engine.getDevice()->findMemoryType(memRequirements.memoryTypeBits, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        if (vkAllocateMemory(engine.getDevice()->getLogicalDevice(), &allocInfo, nullptr, &globalUniformBuffersMemory[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate global uniform buffer memory");
        }

        vkBindBufferMemory(engine.getDevice()->getLogicalDevice(), globalUniformBuffers[i], globalUniformBuffersMemory[i], 0);
        vkMapMemory(engine.getDevice()->getLogicalDevice(), globalUniformBuffersMemory[i], 0, bufferSize, 0, &globalUniformBuffersMapped[i]);
    }
}

void Renderer::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 3> poolSizes{};

    // Global uniform buffers
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // Dynamic uniform buffers
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // Texture samplers - allocate enough for all potential textured models
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_OBJECTS);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT + MAX_OBJECTS);

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
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        // Global uniform buffer
        VkDescriptorBufferInfo globalBufferInfo{};
        globalBufferInfo.buffer = globalUniformBuffers[i];
        globalBufferInfo.offset = 0;
        globalBufferInfo.range = sizeof(GlobalUniformData);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &globalBufferInfo;

        // Dynamic UBO for per-object data
        VkDescriptorBufferInfo dynamicBufferInfo = dynamicUBO->getDescriptorBufferInfo(i);

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &dynamicBufferInfo;

        vkUpdateDescriptorSets(engine.getDevice()->getLogicalDevice(),
                              static_cast<uint32_t>(descriptorWrites.size()),
                              descriptorWrites.data(), 0, nullptr);
    }
}

void Renderer::updateGlobalUniformBuffer(uint32_t currentFrame) {
    GlobalUniformData globalData{};

    // Initialize matrices to identity
    Matrix4 identity = Matrix4::identity();
    memcpy(globalData.view, identity.data(), sizeof(globalData.view));
    memcpy(globalData.proj, identity.data(), sizeof(globalData.proj));
    
    if (camera) {
        Matrix4 view = camera->getViewMatrix();
        Matrix4 proj = camera->getProjectionMatrix();

        memcpy(globalData.view, view.data(), sizeof(globalData.view));
        memcpy(globalData.proj, proj.data(), sizeof(globalData.proj));

        Vector3 viewPos = camera->getPosition();
        globalData.viewPos[0] = viewPos.x;
        globalData.viewPos[1] = viewPos.y;
        globalData.viewPos[2] = viewPos.z;
        globalData.viewPos[3] = 1.0f;
        
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
        globalData.viewPos[0] = 0.0f;
        globalData.viewPos[1] = 0.0f;
        globalData.viewPos[2] = 10.0f;
        globalData.viewPos[3] = 1.0f;
    }
    
    // Set lighting
    if (light) {
        Vector3 lightPos = light->getPosition();
        Vector3 lightColor = light->getColor();

        globalData.lightPos[0] = lightPos.x;
        globalData.lightPos[1] = lightPos.y;
        globalData.lightPos[2] = lightPos.z;
        globalData.lightPos[3] = 1.0f;

        globalData.lightColor[0] = lightColor.x;
        globalData.lightColor[1] = lightColor.y;
        globalData.lightColor[2] = lightColor.z;
        globalData.lightColor[3] = 1.0f;
    } else {
        // Default light
        globalData.lightPos[0] = 2.0f;
        globalData.lightPos[1] = 2.0f;
        globalData.lightPos[2] = 2.0f;
        globalData.lightPos[3] = 1.0f;

        globalData.lightColor[0] = 1.0f;
        globalData.lightColor[1] = 1.0f;
        globalData.lightColor[2] = 1.0f;
        globalData.lightColor[3] = 1.0f;
    }

    memcpy(globalUniformBuffersMapped[currentFrame], &globalData, sizeof(globalData));
}

void Renderer::updateDynamicUBO(uint32_t currentFrame) {
    // Update all model transforms in the dynamic UBO
    for (size_t i = 0; i < models.size(); ++i) {
        if (models[i]) {
            Matrix4 modelMatrix = models[i]->getModelMatrix();
            dynamicUBO->updateObjectTransform(currentFrame, static_cast<uint32_t>(i), modelMatrix);
        }
    }
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

    // Debug output
    static int debugCount = 3;
    bool debug = debugCount > 0;
    if (debug) {
        --debugCount;
        std::cout << "\n=== Dynamic UBO Rendering Debug ===" << std::endl;
        std::cout << "Viewport: " << viewport.width << "x" << viewport.height << std::endl;
        std::cout << "Number of models: " << models.size() << std::endl;

        // Print camera info
        if (camera) {
            Vector3 camPos = camera->getPosition();
            Vector3 camTarget = camera->getTarget();
            std::cout << "Camera Pos: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")" << std::endl;
            std::cout << "Camera Target: (" << camTarget.x << ", " << camTarget.y << ", " << camTarget.z << ")" << std::endl;
        }
    }

    // Track current pipeline to avoid redundant binding
    PipelineType currentPipeline = static_cast<PipelineType>(-1);

    // Render each model with dynamic offsets
    for (size_t i = 0; i < models.size(); ++i) {
        Model* model = models[i];
        if (!model || !model->isVisible()) {
            if (debug) std::cout << "  Model " << i << " skipped (null or invisible)" << std::endl;
            continue;
        }

        // Determine which pipeline to use based on texture coordinates
        PipelineType pipelineType = model->getMesh()->hasTextureCoordinates() ?
                                   PipelineType::TEXTURED : PipelineType::UNTEXTURED;

        // Bind pipeline if it changed
        if (pipelineType != currentPipeline) {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                             pipeline->getPipeline(pipelineType));
            currentPipeline = pipelineType;

            if (debug) {
                std::cout << "  Switched to " << (pipelineType == PipelineType::TEXTURED ? "TEXTURED" : "UNTEXTURED")
                         << " pipeline" << std::endl;
            }
        }

        // Get dynamic offset for this object
        uint32_t dynamicOffset = dynamicUBO->getDynamicOffset(static_cast<uint32_t>(i));

        // Bind descriptor sets with dynamic offset
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pipeline->getPipelineLayout(pipelineType), 0, 1,
                               &descriptorSets[currentFrame], 1, &dynamicOffset);

        // Bind texture descriptor set if this model has a texture
        if (pipelineType == PipelineType::TEXTURED && model->hasTexture()) {
            auto textureIt = textureDescriptorSets.find(model);
            if (textureIt != textureDescriptorSets.end()) {
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                       pipeline->getPipelineLayout(pipelineType), 1, 1,
                                       &textureIt->second, 0, nullptr);

                if (debug) {
                    std::cout << "    Bound texture descriptor set for model" << std::endl;
                }
            }
        }

        // Render this model
        model->render(commandBuffer);

        if (debug) {
            Vector3 pos = model->getPosition();
            Matrix4 modelMatrix = model->getModelMatrix();
            std::cout << "  Model " << i << " at (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
            std::cout << "    Pipeline: " << (pipelineType == PipelineType::TEXTURED ? "TEXTURED" : "UNTEXTURED") << std::endl;
            std::cout << "    Has texture coords: " << (model->getMesh()->hasTextureCoordinates() ? "YES" : "NO") << std::endl;
            std::cout << "    Dynamic offset: " << dynamicOffset << std::endl;
            std::cout << "    Model matrix [0]: " << modelMatrix.data()[0] << ", "
                     << modelMatrix.data()[1] << ", " << modelMatrix.data()[2] << ", " << modelMatrix.data()[3] << std::endl;

            // Show translation part of the matrix (should match model position)
            const float* m = modelMatrix.data();
            std::cout << "    Matrix translation: (" << m[12] << ", " << m[13] << ", " << m[14] << ")" << std::endl;
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}
