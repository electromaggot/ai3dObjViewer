#include "Application.h"
#include "vulkan/VulkanEngine.h"
#include "rendering/Renderer.h"
#include "rendering/Camera.h"
#include "geometry/Model.h"
#include "geometry/ObjLoader.h"
#include "geometry/GeometryGenerator.h"
#include "math/Vector3.h"
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <cstdlib>

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
    loadModels();
    setupScene();
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
    // On macOS, we need to enable the portability subset for MoltenVK
    uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    
    window = SDL_CreateWindow(
        "3D Object Viewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        flags
    );
    
    if (!window) {
        throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
    }
    
    std::cout << "Window created successfully" << std::endl;
    
#ifdef __APPLE__
    // Load Vulkan library explicitly on macOS
    std::cout << "Loading Vulkan library..." << std::endl;
    if (SDL_Vulkan_LoadLibrary(nullptr) != 0) {
        std::cout << "Warning: Failed to load Vulkan library: " << SDL_GetError() << std::endl;
        std::cout << "Trying to continue anyway..." << std::endl;
    } else {
        std::cout << "Vulkan library loaded successfully" << std::endl;
    }
    
    // Test extension enumeration with detailed error checking
    std::cout << "Testing Vulkan extension enumeration..." << std::endl;
    unsigned int testCount = 0;
    
    // First call to get count
    if (!SDL_Vulkan_GetInstanceExtensions(window, &testCount, nullptr)) {
        std::string error = SDL_GetError();
        std::cout << "Error getting extension count: " << error << std::endl;
        
        // Try to provide more helpful error message
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
    
    std::cout << "Found " << testCount << " required Vulkan extensions" << std::endl;
    
    if (testCount == 0) {
        throw std::runtime_error("No Vulkan extensions found. MoltenVK may not be properly installed.");
    }
#endif
}

void Application::initializeVulkan() {
#ifdef __APPLE__
    // Debug: Print environment variables
    std::cout << "=== Vulkan Environment Check ===" << std::endl;
    std::cout << "VULKAN_SDK: " << (getenv("VULKAN_SDK") ? getenv("VULKAN_SDK") : "not set") << std::endl;
    std::cout << "VK_ICD_FILENAMES: " << (getenv("VK_ICD_FILENAMES") ? getenv("VK_ICD_FILENAMES") : "not set") << std::endl;
    std::cout << "VK_LAYER_PATH: " << (getenv("VK_LAYER_PATH") ? getenv("VK_LAYER_PATH") : "not set") << std::endl;
    
    // Check if the MoltenVK ICD file actually exists
    const char* icdPath = getenv("VK_ICD_FILENAMES");
    if (icdPath) {
        std::ifstream file(icdPath);
        if (file.good()) {
            std::cout << "MoltenVK ICD file found and readable" << std::endl;
        } else {
            std::cout << "ERROR: MoltenVK ICD file not found or not readable at: " << icdPath << std::endl;
        }
    }
    std::cout << "=================================" << std::endl;
#endif

    vulkanEngine = std::make_unique<VulkanEngine>(window, windowWidth, windowHeight);
    renderer = std::make_unique<Renderer>(*vulkanEngine);
}

void Application::loadModels() {
    // Create procedural cube
    auto cubeModel = std::make_unique<Model>();
    cubeModel->setMesh(GeometryGenerator::createCube(2.0f));
    cubeModel->setPosition(Vector3(-3.0f, 0.0f, 0.0f));
    models.push_back(std::move(cubeModel));
    
    // Create procedural dodecahedron
    auto dodecahedronModel = std::make_unique<Model>();
    dodecahedronModel->setMesh(GeometryGenerator::createDodecahedron(1.5f));
    dodecahedronModel->setPosition(Vector3(3.0f, 0.0f, 0.0f));
    models.push_back(std::move(dodecahedronModel));
    
    // Try to load OBJ files if they exist
    try {
        ObjLoader loader;
        auto objModel = std::make_unique<Model>();
        objModel->setMesh(loader.load("assets/cube.obj"));
        objModel->setPosition(Vector3(0.0f, 3.0f, 0.0f));
        models.push_back(std::move(objModel));
    } catch (const std::exception& e) {
        std::cout << "Could not load OBJ file: " << e.what() << std::endl;
    }
}

void Application::setupScene() {
    camera = std::make_unique<Camera>();
    
    // Try positioning camera differently - Vulkan uses different conventions
    camera->setPosition(Vector3(0.0f, 0.0f, -5.0f));  // Negative Z to look at positive Z objects
    camera->setTarget(Vector3(0.0f, 0.0f, 0.0f));
    
    // Set perspective with corrected parameters for Vulkan
    float aspect = (float)windowWidth / (float)windowHeight;
    camera->setPerspective(45.0f, aspect, 0.1f, 100.0f);
    
    std::cout << "Setting up scene with camera at (0, 0, -5)" << std::endl;
    
    renderer->setCamera(camera.get());
    
    // Move models closer to origin and make them smaller
    std::cout << "Adding " << models.size() << " models to renderer" << std::endl;
    
    // Recreate models with smaller size and closer positions
    models.clear();
    
    // Create smaller procedural cube
    auto cubeModel = std::make_unique<Model>();
    cubeModel->setMesh(GeometryGenerator::createCube(1.0f));  // Smaller size
    cubeModel->setPosition(Vector3(-2.0f, 0.0f, 0.0f));      // Closer position
    models.push_back(std::move(cubeModel));
    
    // Create smaller dodecahedron
    auto dodecahedronModel = std::make_unique<Model>();
    dodecahedronModel->setMesh(GeometryGenerator::createDodecahedron(0.8f));  // Smaller size
    dodecahedronModel->setPosition(Vector3(2.0f, 0.0f, 0.0f));               // Closer position
    models.push_back(std::move(dodecahedronModel));
    
    // Create smaller sphere for testing
    auto sphereModel = std::make_unique<Model>();
    sphereModel->setMesh(GeometryGenerator::createSphere(0.6f, 16));
    sphereModel->setPosition(Vector3(0.0f, 1.5f, 0.0f));
    models.push_back(std::move(sphereModel));
    
    for (size_t i = 0; i < models.size(); ++i) {
        std::cout << "Adding model " << i << " at position (" 
                  << models[i]->getPosition().x << ", "
                  << models[i]->getPosition().y << ", "
                  << models[i]->getPosition().z << ")" << std::endl;
        renderer->addModel(models[i].get());
    }
    
    std::cout << "Scene setup complete" << std::endl;
}
/*void Application::setupScene() {
    camera = std::make_unique<Camera>();
    camera->setPosition(Vector3(0.0f, 0.0f, 5.0f));  // Moved closer from 10 to 5
    camera->setTarget(Vector3(0.0f, 0.0f, 0.0f));
    
    std::cout << "Setting Perspective for windowWidth " << windowWidth << " windowHeight " << windowHeight << " aspectRatio " << (float) windowWidth / (float) windowHeight << std::endl;

    // Also set a wider field of view to see more
    camera->setPerspective(60.0f, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

    std::cout << "Setting up scene with camera at (0, 0, 5)" << std::endl;
    
    renderer->setCamera(camera.get());
    
    // Add models to renderer
    std::cout << "Adding " << models.size() << " models to renderer" << std::endl;
    for (size_t i = 0; i < models.size(); ++i) {
        std::cout << "Adding model " << i << " at position (" 
                  << models[i]->getPosition().x << ", "
                  << models[i]->getPosition().y << ", "
                  << models[i]->getPosition().z << ")" << std::endl;
        renderer->addModel(models[i].get());
    }
    
    std::cout << "Scene setup complete" << std::endl;
}
void Application::setupScene() {
    camera = std::make_unique<Camera>();
    camera->setPosition(Vector3(0.0f, 0.0f, 10.0f));
    camera->setTarget(Vector3(0.0f, 0.0f, 0.0f));
    
    std::cout << "Setting up scene with camera at (0, 0, 10)" << std::endl;
    
    renderer->setCamera(camera.get());
    
    // Add models to renderer
    std::cout << "Adding " << models.size() << " models to renderer" << std::endl;
    for (size_t i = 0; i < models.size(); ++i) {
        std::cout << "Adding model " << i << " at position (" 
                  << models[i]->getPosition().x << ", "
                  << models[i]->getPosition().y << ", "
                  << models[i]->getPosition().z << ")" << std::endl;
        renderer->addModel(models[i].get());
    }
    
    std::cout << "Scene setup complete" << std::endl;
}*/

void Application::run() {
    running = true;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
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
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    running = false;
                } else {
                    keys[event.key.keysym.scancode] = true;
                }
                break;
                
            case SDL_KEYUP:
                keys[event.key.keysym.scancode] = false;
                break;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    windowWidth = event.window.data1;
                    windowHeight = event.window.data2;
                    vulkanEngine->handleResize(windowWidth, windowHeight);
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
    
    // Camera rotation (basic keyboard rotation)
    if (keys[SDL_SCANCODE_LEFT]) rotation.y += rotateSpeed * deltaTime;
    if (keys[SDL_SCANCODE_RIGHT]) rotation.y -= rotateSpeed * deltaTime;
    if (keys[SDL_SCANCODE_UP]) rotation.x += rotateSpeed * deltaTime;
    if (keys[SDL_SCANCODE_DOWN]) rotation.x -= rotateSpeed * deltaTime;
    
    camera->move(movement);
    camera->rotate(rotation);
    
    // Animate models (simple rotation)
    static float time = 0.0f;
    time += deltaTime;
    
    for (size_t i = 0; i < models.size(); ++i) {
        Vector3 rotation(0.0f, time * 30.0f * (i + 1), 0.0f);
        models[i]->setRotation(rotation);
    }
}

void Application::render() {
    renderer->render();
}

void Application::cleanup() {
    if (vulkanEngine) {
        vulkanEngine->waitIdle();
    }
    
    models.clear();
    camera.reset();
    renderer.reset();
    vulkanEngine.reset();
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    SDL_Quit();
}
