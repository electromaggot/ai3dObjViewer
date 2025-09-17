#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "../rendering/Mesh.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstring>

VulkanPipeline::VulkanPipeline(VulkanDevice& device, VulkanSwapchain& swapchain, VkDescriptorSetLayout descriptorSetLayout)
    : device(device)
    , swapchain(swapchain)
    , descriptorSetLayout(descriptorSetLayout)
    , pipelineLayout(VK_NULL_HANDLE)
    , graphicsPipeline(VK_NULL_HANDLE)
{
    createGraphicsPipeline();
}

VulkanPipeline::~VulkanPipeline() {
    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device.getLogicalDevice(), graphicsPipeline, nullptr);
    }
    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device.getLogicalDevice(), pipelineLayout, nullptr);
    }
}

void VulkanPipeline::createGraphicsPipeline() {
    std::cout << "Creating graphics pipeline..." << std::endl;
    
    // Try to load compiled SPIR-V shaders first, fall back to embedded if not found
    std::vector<uint32_t> vertShaderCode;
    std::vector<uint32_t> fragShaderCode;
    bool usedCompiledShaders = false;
    
    try {
        // Try to load compiled SPIR-V files (matching the CMakeLists.txt output names)
        std::cout << "Attempting to load compiled SPIR-V shaders..." << std::endl;
        std::cout << "  Looking for: shaders/vertex.vert.glsl.spv" << std::endl;
        std::cout << "  Looking for: shaders/fragment.frag.glsl.spv" << std::endl;
        
        auto vertSpirv = readFile("shaders/vertex.vert.glsl.spv");
        auto fragSpirv = readFile("shaders/fragment.frag.glsl.spv");
        
        // Convert to uint32_t vectors
        vertShaderCode.resize(vertSpirv.size() / sizeof(uint32_t));
        fragShaderCode.resize(fragSpirv.size() / sizeof(uint32_t));
        
        memcpy(vertShaderCode.data(), vertSpirv.data(), vertSpirv.size());
        memcpy(fragShaderCode.data(), fragSpirv.data(), fragSpirv.size());
        
        std::cout << "Successfully loaded compiled SPIR-V shaders!" << std::endl;
        usedCompiledShaders = true;
        
    } catch (const std::exception& e) {
        std::cout << "Could not load compiled SPIR-V shaders: " << e.what() << std::endl;
        std::cout << "Falling back to embedded shaders..." << std::endl;
        
        // Use embedded SPIR-V bytecode - basic vertex shader
        // Input: position (vec3), normal (vec3), color (vec3)
        // Output: gl_Position, fragColor
        vertShaderCode = {
            0x07230203, 0x00010000, 0x00080007, 0x0000002c, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
            0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
            0x0008000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000c, 0x0000001e, 0x00000022,
            0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00060005,
            0x0000000c, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000c, 0x00000000,
            0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000c, 0x00000001, 0x505f6c67, 0x746e696f,
            0x657a6953, 0x00000000, 0x00070006, 0x0000000c, 0x00000002, 0x435f6c67, 0x4470696c, 0x61747369,
            0x0065636e, 0x00070006, 0x0000000c, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e,
            0x00030005, 0x0000000e, 0x00000000, 0x00050005, 0x0000001e, 0x6f506e69, 0x69746973, 0x00006e6f,
            0x00050005, 0x00000022, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00050005, 0x00000024, 0x6f43666e,
            0x00726f6c, 0x00050048, 0x0000000c, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000c,
            0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000000c, 0x00000002, 0x0000000b, 0x00000003,
            0x00050048, 0x0000000c, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000c, 0x00000002,
            0x00040047, 0x0000001e, 0x0000001e, 0x00000000, 0x00040047, 0x00000022, 0x0000001e, 0x00000000,
            0x00040047, 0x00000024, 0x0000001e, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
            0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004,
            0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b, 0x00000008, 0x00000009, 0x00000001,
            0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e, 0x0000000c, 0x00000007, 0x00000006,
            0x0000000a, 0x0000000a, 0x00040020, 0x0000000d, 0x00000003, 0x0000000c, 0x0004003b, 0x0000000d,
            0x0000000e, 0x00000003, 0x00040015, 0x0000000f, 0x00000020, 0x00000001, 0x0004002b, 0x0000000f,
            0x00000010, 0x00000000, 0x00040017, 0x00000011, 0x00000006, 0x00000003, 0x00040020, 0x0000001d,
            0x00000001, 0x00000011, 0x0004003b, 0x0000001d, 0x0000001e, 0x00000001, 0x0004002b, 0x00000006,
            0x0000001f, 0x3f800000, 0x00040020, 0x00000021, 0x00000003, 0x00000007, 0x0004003b, 0x00000021,
            0x00000022, 0x00000003, 0x0004003b, 0x0000001d, 0x00000024, 0x00000001, 0x00050036, 0x00000002,
            0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000011, 0x00000020,
            0x0000001e, 0x00050051, 0x00000006, 0x00000025, 0x00000020, 0x00000000, 0x00050051, 0x00000006,
            0x00000026, 0x00000020, 0x00000001, 0x00050051, 0x00000006, 0x00000027, 0x00000020, 0x00000002,
            0x00070050, 0x00000007, 0x00000028, 0x00000025, 0x00000026, 0x00000027, 0x0000001f, 0x00050041,
            0x00000021, 0x00000029, 0x0000000e, 0x00000010, 0x0003003e, 0x00000029, 0x00000028, 0x0004003d,
            0x00000011, 0x00000023, 0x00000024, 0x0003003e, 0x00000022, 0x00000023, 0x000100fd, 0x00010038
        };
        
        // Use embedded SPIR-V bytecode - basic fragment shader
        // Input: fragColor (vec3)
        // Output: outColor (vec4)
        fragShaderCode = {
            0x07230203, 0x00010000, 0x00080007, 0x00000013, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
            0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
            0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010,
            0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d,
            0x00000000, 0x00040005, 0x00000009, 0x436f7475, 0x726f6c6f, 0x00000000, 0x00040005, 0x0000000d,
            0x67617266, 0x6f6c6f43, 0x00000072, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047,
            0x0000000d, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
            0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020,
            0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040017,
            0x0000000a, 0x00000006, 0x00000003, 0x00040020, 0x0000000c, 0x00000001, 0x0000000a, 0x0004003b,
            0x0000000c, 0x0000000d, 0x00000001, 0x0004002b, 0x00000006, 0x0000000f, 0x3f800000, 0x00050036,
            0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000a,
            0x0000000e, 0x0000000d, 0x00050051, 0x00000006, 0x00000010, 0x0000000e, 0x00000000, 0x00050051,
            0x00000006, 0x00000011, 0x0000000e, 0x00000001, 0x00050051, 0x00000006, 0x00000012, 0x0000000e,
            0x00000002, 0x00070050, 0x00000007, 0x00000014, 0x00000010, 0x00000011, 0x00000012, 0x0000000f,
            0x0003003e, 0x00000009, 0x00000014, 0x000100fd, 0x00010038
        };
        
        std::cout << "Using embedded SPIR-V shaders" << std::endl;
    }
    
    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    
    std::cout << "Shader modules created successfully" << std::endl;
    
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // Vertex input
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // Viewport and scissor
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    
    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;  // Counter-clockwise to match OpenGL models
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    
    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;
    
    // Dynamic state
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    
    if (vkCreatePipelineLayout(device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }
    
    std::cout << "Pipeline layout created successfully" << std::endl;
    
    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = swapchain.getRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    
    if (vkCreateGraphicsPipelines(device.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    
    std::cout << "Graphics pipeline created successfully" << std::endl;
    
    vkDestroyShaderModule(device.getLogicalDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(device.getLogicalDevice(), vertShaderModule, nullptr);
}

VkShaderModule VulkanPipeline::createShaderModule(const std::vector<uint32_t>& code) {
    if (code.empty()) {
        throw std::runtime_error("Shader code is empty");
    }
    
    std::cout << "Creating shader module from " << code.size() << " uint32_t words (" << (code.size() * sizeof(uint32_t)) << " bytes)" << std::endl;
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();
    
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device.getLogicalDevice(), &createInfo, nullptr, &shaderModule);
    
    if (result != VK_SUCCESS) {
        std::string errorMsg = "Failed to create shader module. VkResult: ";
        switch (result) {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                errorMsg += "VK_ERROR_OUT_OF_HOST_MEMORY";
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                errorMsg += "VK_ERROR_OUT_OF_DEVICE_MEMORY";
                break;
            case VK_ERROR_INVALID_SHADER_NV:
                errorMsg += "VK_ERROR_INVALID_SHADER_NV (Invalid SPIR-V bytecode)";
                break;
            default:
                errorMsg += "UNKNOWN (" + std::to_string(result) + ")";
                break;
        }
        throw std::runtime_error(errorMsg);
    }
    
    std::cout << "Shader module created successfully" << std::endl;
    return shaderModule;
}

VkShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device.getLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }
    
    return shaderModule;
}

std::vector<char> VulkanPipeline::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filename + " (file not found or not readable)");
    }
    
    size_t fileSize = (size_t) file.tellg();
    if (fileSize == 0) {
        throw std::runtime_error("Shader file is empty: " + filename);
    }
    
    std::cout << "Reading shader file: " << filename << " (" << fileSize << " bytes)" << std::endl;
    
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    
    if (file.gcount() != static_cast<std::streamsize>(fileSize)) {
        throw std::runtime_error("Failed to read complete shader file: " + filename);
    }
    
    file.close();
    
    return buffer;
}
