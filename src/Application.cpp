#include "Application.h"
#include "vulkan/VulkanEngine.h"
#include "rendering/Renderer.h"
#include "rendering/Camera.h"
#include "rendering/Texture.h"
#include "geometry/Model.h"
#include "scene/SceneManager.h"
#include "scene/GeneratedModel.h"
#include "scene/LoadedModel.h"
#include "math/Vector3.h"
#include "utils/logger/Logging.h"
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <cstdlib>

using Shape = GeneratedModel::Shape;


Application::Application()
	: window(nullptr)
	, running(false)
	, windowWidth(1200)
	, windowHeight(800)
{
	memset(keys, 0, sizeof(keys));

	initializeSDL();
	createWindow();
	initializeVulkan();
	setUpScene();

	// Run matrix tests to verify everything is working:
	#if DEBUG_LOW
	Log(LOW, "\n=== Running Matrix Tests ===");
	Camera::testMatrixOperations();
	#endif
}

Application::~Application() {
	cleanup();
}

void Application::initializeSDL() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
	}
}

void Application::createWindow() {
	uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

	window = SDL_CreateWindow(
		"3D Object Viewer - Vulkan",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		flags
	);

	if (!window) {
		throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
	}

	Log(NOTE, "Window created successfully");

#ifdef __APPLE__
	// Load Vulkan library explicitly on macOS
	Log(NOTE, "Loading Vulkan library...");
	if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
		Log(WARN, "Warning: Failed to load Vulkan library: %s", SDL_GetError());
		Log(WARN, "Trying to continue anyway...");
	} else {
		Log(NOTE, "Vulkan library loaded successfully");
	}

	// Test extension enumeration
	unsigned int testCount = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(window, &testCount, nullptr)) {
		std::string error = SDL_GetError();
		Log(ERROR, "Error getting extension count: %s", error.c_str());

		if (error.find("invalid") != std::string::npos) {
			std::string errmsg = "SDL Vulkan extension enumeration failed. This usually means:\n"
								 "1. MoltenVK is not properly installed or configured\n"
								 "2. Environment variables (VK_ICD_FILENAMES) are incorrect\n"
								 "3. The Vulkan loader can't find MoltenVK\n"
								 "Current VK_ICD_FILENAMES: ";
			throw std::runtime_error(errmsg + (getenv("VK_ICD_FILENAMES") ? getenv("VK_ICD_FILENAMES") : "not set"));
		} else {
			throw std::runtime_error("Failed to get Vulkan instance extensions: " + error);
		}
	}

	Log(NOTE, "Found %u required Vulkan extensions", testCount);

	if (testCount == 0) {
		throw std::runtime_error("No Vulkan extensions found. MoltenVK may not be properly installed.");
	}
#endif
}

void Application::initializeVulkan() {
#ifdef __APPLE__
	// Debug: Print environment variables
	Log(NOTE, "=== Vulkan Environment Check ===");
	Log(NOTE, "VULKAN_SDK: %s", (getenv("VULKAN_SDK") ? getenv("VULKAN_SDK") : "not set"));
	Log(NOTE, "VK_ICD_FILENAMES: %s", (getenv("VK_ICD_FILENAMES") ? getenv("VK_ICD_FILENAMES") : "not set"));
	Log(NOTE, "VK_LAYER_PATH: %s", (getenv("VK_LAYER_PATH") ? getenv("VK_LAYER_PATH") : "not set"));

	const char* icdPath = getenv("VK_ICD_FILENAMES");
	if (icdPath) {
		std::ifstream file(icdPath);
		if (file.good()) {
			Log(NOTE, "MoltenVK ICD file found and readable");
		} else {
			Log(ERROR, "ERROR: MoltenVK ICD file not found or not readable at: %s", icdPath);
		}
	}
	Log(NOTE, "=================================");
#endif

	vulkanEngine = std::make_unique<VulkanEngine>(window, windowWidth, windowHeight);
	renderer = std::make_unique<Renderer>(*vulkanEngine);
}


void Application::setUpScene() {
	Log(NOTE, "\n≡≡≡ Setting Up Scene ≡≡≡");

	// Initialize scene manager:
	sceneManager = std::make_unique<SceneManager>();

	// Load scene from JSON file:
	Log(NOTE, "\n=== Loading Scene from JSON ===");
	if (sceneManager->loadFromFile("assets/scenes/default_scene.json")) {
		Log(NOTE, "Scene loaded successfully!");
		Log(NOTE, "Total models loaded: %zu", sceneManager->getObjectCount());
	} else {
		Log(WARN, "Failed to load scene from JSON, falling back to hard-coded scene...");
		createHardcodedFallbackScene();
	}

	// Initialize textures for LoadedModels:
	Log(NOTE, "\n=== Initializing Textures ===");
	for (size_t i = 0; i < sceneManager->getObjectCount(); ++i) {
		SceneObject* obj = sceneManager->getObject(i);
		if (obj && obj->getType() == SceneObject::ObjectType::LOADED_MODEL) {
			LoadedModel* loadedModel = static_cast<LoadedModel*>(obj);
			loadedModel->initializeTexture(*vulkanEngine->getDevice(), *vulkanEngine);
		}
	}

	// Create models for rendering:
	models = sceneManager->createAllModels();

	// Setup camera:
	camera = std::make_unique<Camera>();
	camera->setPosition(Vector3(0.0f, 2.0f, 8.0f));
	camera->setTarget(Vector3(0.0f, 0.0f, 0.0f));
	camera->setUp(Vector3(0.0f, 1.0f, 0.0f));
	camera->rotate(Vector3(16.0f, 90.0f, 0.0f)); // Temporary hack

	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	camera->setPerspective(45.0f, aspect, 0.1f, 100.0f);

	Log(NOTE, "Camera setup:");
	Log(NOTE, "  Position: (0, 2, -8)");
	Log(NOTE, "  Target: (0, 0, 0)");
	Log(NOTE, "  FOV: 45°, Aspect: %f", aspect);

	camera->debugPrintMatrices();
	renderer->setCamera(camera.get());

	// Add models to renderer:
	Log(NOTE, "\nAdding %zu models to renderer:", models.size());
	for (size_t i = 0; i < models.size(); ++i) {
		if (models[i]) {
			Vector3 pos = models[i]->getPosition();
			Log(NOTE, "  Model %zu at position (%f, %f, %f)", i, pos.x, pos.y, pos.z);
			renderer->addModel(models[i].get());
		}
	}


	Log(NOTE, "\nScene setup complete!\n"
			  "=================================");
	Log(NOTE, "Controls:\n"
			  "  WASD: Move horizontally\n"
			  "  QE: Move up/down\n"
			  "  Arrow Keys: Look around\n"
			  "  ESC: Exit\n"
			  "  P: Toggle perspective/orthographic\n"
			  "  R: Reset camera\n"
			  "  Space: Stop/start animation\n"
			  "=================================");
}

void Application::run() {
	running = true;
	animationPaused = false;

	auto lastTime = std::chrono::high_resolution_clock::now();
	int frameCount = 0;
	auto fpsTime = lastTime;

	while (running) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		frameCount++;	// FPS counter:
		float fpsDelta = std::chrono::duration<float>(currentTime - fpsTime).count();
		if (fpsDelta >= 1.0f) {
			float fps = frameCount / fpsDelta;
			SDL_SetWindowTitle(window, ("3D Object Viewer - Vulkan [FPS: " + std::to_string(static_cast<int>(fps)) + "]").c_str());
			frameCount = 0;
			fpsTime = currentTime;
		}

		handleEvents();
		update(deltaTime);
		render();
	}
}

void Application::handleEvents() {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_ESCAPE:
						running = false;
						break;

					case SDL_SCANCODE_P:		// Toggle perspective/orthographic.
						if (!keys[SDL_SCANCODE_P]) {  // Prevent key repeat.
							toggleProjectionMode();
						}
						break;

					case SDL_SCANCODE_R:		// Reset camera
						resetCamera();
						break;

					case SDL_SCANCODE_SPACE:	// Toggle animation
						if (!keys[SDL_SCANCODE_SPACE]) {  // Prevent key repeat.
							animationPaused = !animationPaused;
							Log(NOTE, "Animation %s", (animationPaused ? "paused" : "resumed"));
						}
						break;

					default:
						break;
				}
				keys[event.key.keysym.scancode] = true;
				break;

			case SDL_KEYUP:
				keys[event.key.keysym.scancode] = false;
				break;

			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					windowWidth = event.window.data1;
					windowHeight = event.window.data2;
					float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
					camera->setAspectRatio(aspect);
					vulkanEngine->handleResize(windowWidth, windowHeight);
					Log(NOTE, "Window resized to %u × %u", windowWidth, windowHeight);
				}
				break;
		}
	}
}

void Application::update(float deltaTime) {
	const float moveSpeed = 5.0f;
	const float rotateSpeed = 90.0f; // degrees per second

	Vector3 movement(0.0f, 0.0f, 0.0f);
	Vector3 rotation(0.0f, 0.0f, 0.0f);

	// Camera movement
	if (keys[SDL_SCANCODE_W]) movement.z -= moveSpeed * deltaTime;
	if (keys[SDL_SCANCODE_S]) movement.z += moveSpeed * deltaTime;
	if (keys[SDL_SCANCODE_A]) movement.x -= moveSpeed * deltaTime;
	if (keys[SDL_SCANCODE_D]) movement.x += moveSpeed * deltaTime;
	if (keys[SDL_SCANCODE_Q]) movement.y += moveSpeed * deltaTime;
	if (keys[SDL_SCANCODE_E]) movement.y -= moveSpeed * deltaTime;

	// Camera rotation
	if (keys[SDL_SCANCODE_LEFT]) rotation.y += rotateSpeed * deltaTime;
	if (keys[SDL_SCANCODE_RIGHT]) rotation.y -= rotateSpeed * deltaTime;
	if (keys[SDL_SCANCODE_UP]) rotation.x += rotateSpeed * deltaTime;
	if (keys[SDL_SCANCODE_DOWN]) rotation.x -= rotateSpeed * deltaTime;

	camera->move(movement);
	camera->rotate(rotation);

	// Animate models (simple rotation)
	if (!animationPaused) {
		static float time = 0.0f;
		time += deltaTime;

		// Different rotation speeds for different models.
		for (size_t i = 0; i < models.size(); ++i) {
			if (i == 4) continue;  // Don't rotate the ground plane
			if (i == 6) continue;  // Don't rotate the viking room (textured model)

			float rotationSpeed = 30.0f * (1.0f + i * 0.5f);  // Vary speed by model
			Vector3 rotation(
				i == 3 ? time * 20.0f : 0.0f, // Cylinder rotates on X
				time * rotationSpeed,		  // All rotate on Y
				0.0f
			);
			models[i]->setRotation(rotation);
		}
	}
}

void Application::render() {
	renderer->render();
}

void Application::toggleProjectionMode() {
	static bool isPerspective = true;
	isPerspective = !isPerspective;

	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

	if (isPerspective) {
		camera->setPerspective(45.0f, aspect, 0.1f, 100.0f);
		Log(NOTE, "Switched to perspective projection");
	} else {
		camera->setOrthographicByHeight(10.0f, 0.1f, 100.0f);
		Log(NOTE, "Switched to orthographic projection");
	}
}

void Application::resetCamera() {
	camera->setPosition(Vector3(0.0f, 2.0f, 8.0f));
	camera->setTarget(Vector3(0.0f, 0.0f, 0.0f));
	camera->setUp(Vector3(0.0f, 1.0f, 0.0f));

	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	camera->setPerspective(45.0f, aspect, 0.1f, 100.0f);

	Log(NOTE, "Camera reset to default position");
}

void Application::cleanup() {
	if (vulkanEngine) {
		vulkanEngine->waitIdle();
	}

	// Clear models first while VulkanDevice is still valid.
	// This ensures Mesh destructors can properly clean up Vulkan buffers.
	models.clear();

	// Clear scene manager to release any cached meshes.
	sceneManager.reset();

	camera.reset();
	renderer.reset();
	vulkanEngine.reset();

	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	SDL_Quit();
}

void Application::createHardcodedFallbackScene() {	// just in case!
	// Failsafe create scene objects using the old hard-coded system.
	Log(NOTE, "\n=== Creating Scene Objects (Fallback) ===");

	sceneManager->addGeneratedModel(Shape::CUBE, Vector3(-3.0f, 0.0f, 0.0f), 1.0f);
	Log(NOTE, "Created cube at (-3, 0, 0)");

	sceneManager->addGeneratedModel(Shape::SPHERE, Vector3(0.0f, 2.0f, 0.0f), 0.8f, 0.0f, 24);
	Log(NOTE, "Created sphere at (0, 2, 0)");

	sceneManager->addGeneratedModel(Shape::DODECAHEDRON, Vector3(3.0f, 0.0f, 0.0f), 0.9f);
	Log(NOTE, "Created dodecahedron at (3, 0, 0)");

	sceneManager->addGeneratedModel(Shape::CYLINDER, Vector3(0.0f, 0.0f, -3.0f), 0.5f, 1.5f, 20);
	Log(NOTE, "Created cylinder at (0, 0, -3)");

	sceneManager->addGeneratedModel(Shape::PLANE, Vector3(0.0f, -2.0f, 0.0f), 8.0f, 8.0f);
	Log(NOTE, "Created plane at (0, -2, 0)");

	try {
		sceneManager->addLoadedModel("OBJ Cube", Vector3(0.0f, -1.0f, 0.0f), "assets/models/cube.obj");
		Log(NOTE, "Loaded OBJ cube at (0, -1, 0)");
	} catch (const std::exception& e) {
		Log(WARN, "Could not load OBJ file: %s", e.what());
	}
	try {
		sceneManager->addLoadedModel("Viking Room", Vector3(0.0f, -1.9f, 3.0f), "assets/models/viking_room.obj");
		Log(NOTE, "Loaded OBJ viking room at (0, -1.9, 3)");
	} catch (const std::exception& e) {
		Log(WARN, "Could not load OBJ file: %s", e.what());
	}
	Log(NOTE, "Total models created: %zu", sceneManager->getObjectCount());
}

