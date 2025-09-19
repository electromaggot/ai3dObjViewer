#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class VulkanEngine;
class Renderer;
class Camera;
class Model;
class SceneManager;

class Application {
public:
	Application();
	~Application();

	void run();

private:
	void initializeSDL();
	void createWindow();
	void initializeVulkan();
	void setUpScene();

	void mainLoop();
	void handleEvents();
	void toggleProjectionMode();
	void resetCamera();
	void update(float deltaTime);
	void render();
	void cleanup();

	// SDL components
	SDL_Window* window;

	// Vulkan and rendering
	std::unique_ptr<VulkanEngine> vulkanEngine;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<Camera> camera;

	// Scene management
	std::unique_ptr<SceneManager> sceneManager;
	std::vector<std::unique_ptr<Model>> models;  // Cached models for rendering

	// Application state
	bool running;
	uint32_t windowWidth;
	uint32_t windowHeight;

	bool animationPaused;  // Control animation state

	// Input state
	bool keys[SDL_NUM_SCANCODES];

	// (just in case)
	void createHardcodedFallbackScene();
};
