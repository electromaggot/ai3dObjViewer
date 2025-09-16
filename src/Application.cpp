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

    // Run matrix tests to verify everything is working
    std::cout << "\n=== Running Matrix Tests ===" << std::endl;
    Camera::testMatrixOperations();
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

    // Test extension enumeration
    unsigned int testCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &testCount, nullptr)) {
        std::string error = SDL_GetError();
        std::cout << "Error getting extension count: " << error << std::endl;

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
    std::cout << "\n=== Loading Models ===" << std::endl;

    // Create a variety of test models to verify rendering

    // 1. Cube - Basic test shape
    auto cubeModel = std::make_unique<Model>();
    cubeModel->setMesh(GeometryGenerator::createCube(1.0f));
    cubeModel->setPosition(Vector3(-3.0f, 0.0f, 0.0f));
    models.push_back(std::move(cubeModel));
    std::cout << "Created cube at (-3, 0, 0)" << std::endl;

    // 2. Sphere - Tests curved surfaces
    auto sphereModel = std::make_unique<Model>();
    sphereModel->setMesh(GeometryGenerator::createSphere(0.8f, 24));
    sphereModel->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    models.push_back(std::move(sphereModel));
    std::cout << "Created sphere at (0, 0, 0)" << std::endl;

    // 3. Dodecahedron - Complex geometry
    auto dodecahedronModel = std::make_unique<Model>();
    dodecahedronModel->setMesh(GeometryGenerator::createDodecahedron(0.9f));
    dodecahedronModel->setPosition(Vector3(3.0f, 0.0f, 0.0f));
    models.push_back(std::move(dodecahedronModel));
    std::cout << "Created dodecahedron at (3, 0, 0)" << std::endl;

    // 4. Cylinder - Different primitive type
    auto cylinderModel = std::make_unique<Model>();
    cylinderModel->setMesh(GeometryGenerator::createCylinder(0.5f, 1.5f, 20));
    cylinderModel->setPosition(Vector3(0.0f, 0.0f, -3.0f));
    models.push_back(std::move(cylinderModel));
    std::cout << "Created cylinder at (0, 0, -3)" << std::endl;

    // 5. Plane - Ground reference
    auto planeModel = std::make_unique<Model>();
    planeModel->setMesh(GeometryGenerator::createPlane(8.0f, 8.0f));
    planeModel->setPosition(Vector3(0.0f, -2.0f, 0.0f));
    models.push_back(std::move(planeModel));
    std::cout << "Created plane at (0, -2, 0)" << std::endl;

    // 6. Try to load OBJ file if it exists
    try {
        ObjLoader loader;
        auto objModel = std::make_unique<Model>();
        objModel->setMesh(loader.load("assets/cube.obj"));
        objModel->setPosition(Vector3(0.0f, 2.0f, 0.0f));
        models.push_back(std::move(objModel));
        std::cout << "Loaded OBJ cube at (0, 2, 0)" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Could not load OBJ file: " << e.what() << std::endl;
        std::cout << "Continuing without OBJ model..." << std::endl;
    }

    std::cout << "Total models created: " << models.size() << std::endl;
}

void Application::setupScene() {
    std::cout << "\n≡≡≡ Setting Up Scene ≡≡≡" << std::endl;

    camera = std::make_unique<Camera>();

    // Position camera back along +Z axis, looking toward origin
    camera->setPosition(Vector3(0.0f, 2.0f, -8.0f));
    camera->setTarget(Vector3(0.0f, 0.0f, 0.0f));
    camera->setUp(Vector3(0.0f, 1.0f, 0.0f));

    // Add yaw rotation of -90 degrees to mimic manual turn necessary to center view to top of scene:
    camera->rotate(Vector3(0.0f, -90.0f, 0.0f));

    // Set perspective with proper aspect ratio
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    camera->setPerspective(45.0f, aspect, 0.1f, 100.0f);

    std::cout << "Camera setup:" << std::endl;
    std::cout << "  Position: (0, 2, -8)" << std::endl;
    std::cout << "  Target: (0, 0, 0)" << std::endl;
    std::cout << "  Looking direction: along +Z (toward origin)" << std::endl;
    std::cout << "  FOV: 45°" << std::endl;
    std::cout << "  Aspect: " << aspect << " (" << windowWidth << "x" << windowHeight << ")" << std::endl;

    // Print initial camera matrices for debugging
    camera->debugPrintMatrices();

    renderer->setCamera(camera.get());

    // Add all models to the renderer
    std::cout << "\nAdding " << models.size() << " models to renderer:" << std::endl;
    for (size_t i = 0; i < models.size(); ++i) {
        if (models[i]) {
            Vector3 pos = models[i]->getPosition();
            std::cout << "  Model " << i << " at position (" 
                      << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
            renderer->addModel(models[i].get());
        }
    }

    std::cout << "\nScene setup complete!" << std::endl;
    std::cout << "=================================" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  WASD: Move horizontally" << std::endl;
    std::cout << "  QE: Move up/down" << std::endl;
    std::cout << "  Arrow Keys: Look around" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << "  P: Toggle perspective/orthographic" << std::endl;
    std::cout << "  R: Reset camera" << std::endl;
    std::cout << "  Space: Stop/start animation" << std::endl;
    std::cout << "=================================" << std::endl;
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

        // FPS counter
        frameCount++;
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

                    case SDL_SCANCODE_P:
                        // Toggle perspective/orthographic
                        if (!keys[SDL_SCANCODE_P]) {  // Prevent key repeat
                            toggleProjectionMode();
                        }
                        break;

                    case SDL_SCANCODE_R:
                        // Reset camera
                        resetCamera();
                        break;

                    case SDL_SCANCODE_SPACE:
                        // Toggle animation
                        if (!keys[SDL_SCANCODE_SPACE]) {  // Prevent key repeat
                            animationPaused = !animationPaused;
                            std::cout << "Animation " << (animationPaused ? "paused" : "resumed") << std::endl;
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
                    std::cout << "Window resized to " << windowWidth << "x" << windowHeight << std::endl;
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

        // Different rotation speeds for different models
        for (size_t i = 0; i < models.size(); ++i) {
            if (i == 4) continue;  // Don't rotate the ground plane

            float rotationSpeed = 30.0f * (1.0f + i * 0.5f);  // Vary speed by model
            Vector3 rotation(
                i == 3 ? time * 20.0f : 0.0f,  // Cylinder rotates on X
                time * rotationSpeed,           // All rotate on Y
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
        std::cout << "Switched to perspective projection" << std::endl;
    } else {
        camera->setOrthographicByHeight(10.0f, 0.1f, 100.0f);
        std::cout << "Switched to orthographic projection" << std::endl;
    }
}

void Application::resetCamera() {
    camera->setPosition(Vector3(0.0f, 2.0f, 8.0f));
    camera->setTarget(Vector3(0.0f, 0.0f, 0.0f));
    camera->setUp(Vector3(0.0f, 1.0f, 0.0f));

    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    camera->setPerspective(45.0f, aspect, 0.1f, 100.0f);

    std::cout << "Camera reset to default position" << std::endl;
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

