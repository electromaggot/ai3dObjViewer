# 3dObjViewer - Project Overview

## Project Description
A cross-platform 3D model viewer built with C++ using Vulkan for rendering and SDL2 for window management and input handling. The project is designed with extensibility and clean architecture in mind.

## Architecture & Design Principles
- **Object-Oriented Design**: Small, focused classes with single responsibilities
- **SOLID Principles**: Easy to extend and modify components
- **RAII**: Automatic resource management
- **DRY**: Reusable components and utilities
- **Separation of Concerns**: Vulkan rendering isolated from application logic

## Project Structure
```
3dObjViewer/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── Application.cpp/.h          # Main application class
│   ├── vulkan/                     # Vulkan rendering engine
│   │   ├── VulkanEngine.cpp/.h     # Main Vulkan wrapper
│   │   ├── VulkanDevice.cpp/.h     # Device and queue management
│   │   ├── VulkanSwapchain.cpp/.h  # Swapchain management
│   │   ├── VulkanBuffer.cpp/.h     # Buffer creation and management
│   │   ├── VulkanImage.cpp/.h      # Image and texture management
│   │   ├── VulkanPipeline.cpp/.h   # Graphics pipeline management
│   │   └── VulkanUtils.cpp/.h      # Utility functions
│   ├── rendering/                  # High-level rendering components
│   │   ├── Renderer.cpp/.h         # Main renderer
│   │   ├── Camera.cpp/.h           # Camera system
│   │   ├── Light.cpp/.h            # Lighting system
│   │   └── Mesh.cpp/.h             # Mesh representation
│   ├── geometry/                   # 3D geometry and models
│   │   ├── Model.cpp/.h            # 3D model representation
│   │   ├── ObjLoader.cpp/.h        # OBJ file loader
│   │   └── GeometryGenerator.cpp/.h # Procedural geometry
│   ├── math/                       # Mathematics utilities
│   │   ├── Vector3.cpp/.h          # 3D vector math
│   │   ├── Matrix4.cpp/.h          # 4x4 matrix operations
│   │   └── Transform.cpp/.h        # Transform utilities
│   └── utils/                      # General utilities
│       ├── FileUtils.cpp/.h        # File I/O utilities
│       └── Logger.cpp/.h           # Logging system
├── shaders/                        # GLSL shader files
│   ├── vertex.glsl                 # Vertex shader
│   └── fragment.glsl               # Fragment shader
├── assets/                         # 3D models and resources
│   ├── cube.obj                    # Example cube model
│   └── dodecahedron.obj            # Example dodecahedron model
├── CMakeLists.txt                  # Build configuration
└── setupFiles.sh                   # File organization script
```

## Key Features
1. **Multi-platform Support**: macOS primary, Windows/Linux secondary
2. **Vulkan Rendering**: Modern graphics API with proper abstraction
3. **Model Loading**: OBJ file format support
4. **Procedural Geometry**: Runtime geometry generation
5. **Camera System**: Free-look camera with keyboard controls
6. **Lighting Model**: Basic Phong lighting (extensible)
7. **Multiple Object Support**: Render multiple models at different positions

## Dependencies
- **SDL2**: Window management and input
- **Vulkan SDK**: Graphics API
- **GLM**: Mathematics library (header-only)
- **CMake**: Build system

## Build Instructions
1. Install dependencies (Vulkan SDK, SDL2, GLM)
2. Run `mkdir build && cd build`
3. Run `cmake ..`
4. Run `make` (or build with your preferred method)

## Controls
- **WASD**: Move camera forward/left/back/right
- **QE**: Move camera up/down
- **Mouse**: Look around (when implemented)
- **ESC**: Exit application

## Extension Points
- **Rendering**: Easy to add new shaders and rendering techniques
- **Geometry**: Simple to add new model formats or procedural generators
- **Lighting**: Modular lighting system for different lighting models
- **Camera**: Base camera class for different camera behaviors
- **Materials**: Ready for texture and material system expansion

## Development Notes
- Use 4-space tabs for indentation
- Follow CamelCase naming convention
- Each Vulkan component is wrapped in its own class
- Resource management follows RAII principles
- All classes designed for inheritance and extension

## Future Iterations
- Texture support
- Advanced lighting models (PBR)
- Animation system
- Scene graph
- GUI integration
- Mobile platform support
- Multi-threaded rendering