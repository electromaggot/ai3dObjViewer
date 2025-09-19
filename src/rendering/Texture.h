#pragma once

#include <vulkan/vulkan.h>
#include <string>

class VulkanDevice;

class Texture {
public:
	Texture();
	~Texture();

	bool loadFromFile(const std::string& filename, VulkanDevice& device, class VulkanEngine& engine, bool flipVertically = false);
	void cleanup();

	VkImage getImage() const { return textureImage; }
	VkImageView getImageView() const { return textureImageView; }
	VkSampler getSampler() const { return textureSampler; }

	bool isLoaded() const { return loaded; }

	// Static default texture for models without textures
	static Texture* getDefaultTexture(VulkanDevice& device, class VulkanEngine& engine);

private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VulkanDevice* device;
	class VulkanEngine* engine;
	bool loaded;

	static Texture* defaultTexture;

	bool createDefaultWhiteTexture();
	void createTextureImage(unsigned char* pixels, int width, int height);
	std::vector<unsigned char> flipImageVertically(unsigned char* pixels, int width, int height, int bytesPerPixel);
	void createTextureImageView();
	void createTextureSampler();
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};