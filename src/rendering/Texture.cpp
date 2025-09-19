#include "Texture.h"
#include "../vulkan/VulkanDevice.h"
#include "../vulkan/VulkanEngine.h"
#include "../vulkan/VulkanBuffer.h"
#include <SDL2/SDL_image.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

Texture* Texture::defaultTexture = nullptr;

Texture::Texture()
    : textureImage(VK_NULL_HANDLE)
    , textureImageMemory(VK_NULL_HANDLE)
    , textureImageView(VK_NULL_HANDLE)
    , textureSampler(VK_NULL_HANDLE)
    , device(nullptr)
    , engine(nullptr)
    , loaded(false)
{
}

Texture::~Texture() {
    cleanup();
}

bool Texture::loadFromFile(const std::string& filename, VulkanDevice& vulkanDevice, VulkanEngine& vulkanEngine) {
    this->device = &vulkanDevice;
    this->engine = &vulkanEngine;

    std::cout << "Loading texture: " << filename;

    // Check if this is the special default texture case
    if (filename == "default_white") {
        std::cout << " (creating default white texture)" << std::endl;
        return createDefaultWhiteTexture();
    }

    // Try to load actual image file using SDL_image
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        std::cout << " - Failed: " << IMG_GetError() << std::endl;
        std::cout << "Creating white placeholder texture instead" << std::endl;
        return createDefaultWhiteTexture();
    }

    std::cout << " - Success! (" << surface->w << "x" << surface->h << ", format: " << SDL_GetPixelFormatName(surface->format->format) << ")" << std::endl;

    // Convert to ABGR8888 if necessary (matches VK_FORMAT_R8G8B8A8_SRGB byte order)
    SDL_Surface* rgbaSurface = nullptr;
    if (surface->format->format != SDL_PIXELFORMAT_ABGR8888) {
        std::cout << "Converting from " << SDL_GetPixelFormatName(surface->format->format) << " to ABGR8888" << std::endl;
        rgbaSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
        SDL_FreeSurface(surface);
        if (!rgbaSurface) {
            std::cerr << "Failed to convert texture to ABGR: " << SDL_GetError() << std::endl;
            return createDefaultWhiteTexture();
        }
        surface = rgbaSurface;
        std::cout << "Conversion successful, new format: " << SDL_GetPixelFormatName(surface->format->format) << std::endl;
    } else {
        std::cout << "Image already in ABGR8888 format" << std::endl;
    }

    try {
        createTextureImage(static_cast<unsigned char*>(surface->pixels), surface->w, surface->h);
        createTextureImageView();
        createTextureSampler();
        loaded = true;

        SDL_FreeSurface(surface);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create Vulkan texture: " << e.what() << std::endl;
        SDL_FreeSurface(surface);
        return false;
    }
}

void Texture::cleanup() {
    if (device && loaded) {
        VkDevice logicalDevice = device->getLogicalDevice();

        if (textureSampler != VK_NULL_HANDLE) {
            vkDestroySampler(logicalDevice, textureSampler, nullptr);
            textureSampler = VK_NULL_HANDLE;
        }

        if (textureImageView != VK_NULL_HANDLE) {
            vkDestroyImageView(logicalDevice, textureImageView, nullptr);
            textureImageView = VK_NULL_HANDLE;
        }

        if (textureImage != VK_NULL_HANDLE) {
            vkDestroyImage(logicalDevice, textureImage, nullptr);
            textureImage = VK_NULL_HANDLE;
        }

        if (textureImageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(logicalDevice, textureImageMemory, nullptr);
            textureImageMemory = VK_NULL_HANDLE;
        }

        loaded = false;
    }
}

Texture* Texture::getDefaultTexture(VulkanDevice& vulkanDevice, VulkanEngine& vulkanEngine) {
    if (!defaultTexture) {
        defaultTexture = new Texture();
        if (!defaultTexture->loadFromFile("default_white", vulkanDevice, vulkanEngine)) {
            delete defaultTexture;
            defaultTexture = nullptr;
            throw std::runtime_error("Failed to create default texture");
        }
    }
    return defaultTexture;
}

bool Texture::createDefaultWhiteTexture() {
    // Create a simple 2x2 white texture
    const int width = 2;
    const int height = 2;
    const int channels = 4; // RGBA
    unsigned char pixels[width * height * channels];

    // Fill with white color
    for (int i = 0; i < width * height * channels; i += channels) {
        pixels[i] = 255;     // R
        pixels[i + 1] = 255; // G
        pixels[i + 2] = 255; // B
        pixels[i + 3] = 255; // A
    }

    try {
        createTextureImage(pixels, width, height);
        createTextureImageView();
        createTextureSampler();
        loaded = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create default white texture: " << e.what() << std::endl;
        return false;
    }
}

void Texture::createTextureImage(unsigned char* pixels, int width, int height) {
    VkDeviceSize imageSize = width * height * 4; // 4 bytes per pixel (RGBA)

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device->getLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device->getLogicalDevice(), stagingBufferMemory);

    // Create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device->getLogicalDevice(), &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->getLogicalDevice(), textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate texture image memory");
    }

    vkBindImageMemory(device->getLogicalDevice(), textureImage, textureImageMemory, 0);

    // Transfer the texture data to GPU
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device->getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device->getLogicalDevice(), stagingBufferMemory, nullptr);
}

void Texture::createTextureImageView() {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view");
    }
}

void Texture::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device->getLogicalDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler");
    }
}

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    // Create a temporary command buffer
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}

void Texture::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // Create buffer using Vulkan directly
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->getLogicalDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory");
    }

    vkBindBufferMemory(device->getLogicalDevice(), buffer, bufferMemory, 0);
}

uint32_t Texture::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    return device->findMemoryType(typeFilter, properties);
}

VkCommandBuffer Texture::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = engine->getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->getLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Texture::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->getGraphicsQueue());

    vkFreeCommandBuffers(device->getLogicalDevice(), engine->getCommandPool(), 1, &commandBuffer);
}