#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <memory>

class VulkanDevice;
class VulkanSwapchain;
class VulkanBuffer;

class VulkanEngine {
public:
	VulkanEngine(SDL_Window* window, uint32_t width, uint32_t height);
	~VulkanEngine();

	void handleResize(uint32_t width, uint32_t height);
	void waitIdle();

	// Getters for Vulkan objects
	VkInstance getInstance() const { return instance; }
	VkSurfaceKHR getSurface() const { return surface; }
	VulkanDevice* getDevice() const { return device.get(); }
	VulkanSwapchain* getSwapchain() const { return swapchain.get(); }
	VkCommandPool getCommandPool() const { return commandPool; }

	uint32_t getCurrentImageIndex() const { return imageIndex; }

	VkCommandBuffer beginFrame();
	void endFrame(VkCommandBuffer commandBuffer);

private:
	void createInstance();
	void createSurface();
	void createDevice();
	void createSwapchain();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronizationObjects();

	SDL_Window* window;

	VkInstance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;

	std::unique_ptr<VulkanDevice> device;
	std::unique_ptr<VulkanSwapchain> swapchain;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	uint32_t currentFrame;
	uint32_t imageIndex;

	static const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef _DEBUG
	void setupDebugMessenger();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
#endif
};
