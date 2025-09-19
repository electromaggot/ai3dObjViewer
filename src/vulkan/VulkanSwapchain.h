#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class VulkanDevice;

class VulkanSwapchain {
public:
	VulkanSwapchain(VulkanDevice& device, VkSurfaceKHR surface, uint32_t width, uint32_t height);
	~VulkanSwapchain();

	void recreate(uint32_t width, uint32_t height);

	VkSwapchainKHR getSwapchain() const { return swapchain; }
	VkFormat getImageFormat() const { return imageFormat; }
	VkExtent2D getExtent() const { return extent; }
	VkRenderPass getRenderPass() const { return renderPass; }
	VkFramebuffer getFramebuffer(uint32_t index) const { return framebuffers[index]; }
	uint32_t getImageCount() const { return static_cast<uint32_t>(images.size()); }

private:
	void createSwapchain();
	void createImageViews();
	void createRenderPass();
	void createDepthResources();
	void createFramebuffers();

	void cleanup();

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkFormat findDepthFormat();

	VulkanDevice& device;
	VkSurfaceKHR surface;
	uint32_t width, height;

	VkSwapchainKHR swapchain;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	VkFormat imageFormat;
	VkExtent2D extent;

	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
};