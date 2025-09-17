# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Initial setup (from project root)
mkdir build
cd build
cmake ..

# Build the project
make

# Build with parallel jobs
make -j8

# Clean build
make clean

# Rebuild from scratch
rm -rf build && mkdir build && cd build && cmake .. && make

# Compile shaders only
make CompileShaders

# Run the application (from build directory)
./3dObjViewer
```

## Architecture Overview

This is a Vulkan-based 3D object viewer with a layered architecture:

1. **Vulkan Layer** (`src/vulkan/`): Low-level Vulkan abstractions
   - `VulkanEngine`: Main Vulkan initialization and management
   - `VulkanDevice`: Physical/logical device and queue management
   - `VulkanSwapchain`: Presentation surface management
   - `VulkanBuffer`: Vertex/index/uniform buffer abstractions
   - `VulkanPipeline`: Graphics pipeline configuration

2. **Rendering Layer** (`src/rendering/`): High-level rendering components
   - `Renderer`: Coordinates rendering operations, manages per-object transforms
   - `Camera`: View/projection matrix generation with FPS-style controls
   - `DynamicUBO`: Dynamic uniform buffer management for per-object transforms
   - `Object`: Represents renderable objects with world positions
   - `Mesh`: Vertex/index data container

3. **Geometry Layer** (`src/geometry/`): 3D model loading and generation
   - `Model`: High-level model representation
   - `ObjLoader`: Parses .obj files from `assets/` directory
   - `GeometryGenerator`: Creates procedural geometry (cubes, spheres)

4. **Math Layer** (`src/math/`): Custom math implementations
   - Uses column-major matrices for Vulkan compatibility
   - Matrix multiplication order: projection * view * model

## Key Technical Details

### Shader System
- Shaders in `shaders/` directory use `.vert.glsl` and `.frag.glsl` extensions
- Compiled to SPIR-V during build via `glslangValidator`
- Vertex shader expects: position (vec3), normal (vec3), color (vec3)
- Uniform buffer: separate matrices for model, view, projection

### Rendering Pipeline
- Multiple objects share vertex/index buffers but have unique world transforms
- Dynamic UBO system allocates per-object uniform data
- Push constants used for per-object model matrices
- Fixed camera controls: WASD (move), QE (up/down), Arrow keys (rotate)

### Current Active Development (dynamicUBO branch)
- Implementing dynamic uniform buffer objects for per-model transforms
- New files: `DynamicUBO.cpp/h`, `Object.cpp/h`
- Modified: `Renderer.cpp/h` for per-object rendering

## Known Issues & Notes

- The project was AI-generated as a Vulkan learning experiment
- Rendering issues may exist with projection matrices or transform calculations
- The codebase prioritizes clean architecture over optimization
- SDL2 is used for windowing and input, not rendering

## Testing

Currently no automated tests. Manual testing involves:
1. Building the project
2. Running `./3dObjViewer` from build directory
3. Testing camera controls and object rendering
4. Loading different .obj files from assets directory