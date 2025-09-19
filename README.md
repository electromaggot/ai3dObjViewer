# 3D Object Viewer

A cross-platform 3D model viewer built with C++, Vulkan, and SDL2.

(This project was created by AI (Claude) as an experiment, to test how well it knows
 Vulkan and its ability generate code "from scratch" similar to my other repos.
 The "ask" is complex, as is Vulkan big, leading to challenges of a large context.
 Code quality is important, but moreso, how good do its graphics look on-screen?

 Arriving at this codebase, as I have it first checked-in, has been an ordeal; the
 iterations were numerous.  And these initial versions still don't work correctly.
 Perhaps Claude doesn't understand Projection Matrices.  So if you're a human
 reading this, there isn't much to see here, so you can move along!)

## Building

### Prerequisites
- CMake 3.16+
- Vulkan SDK
- SDL2
- C++17 compatible compiler

### Build Steps
```bash
mkdir build
cd build
cmake ..
make  # or build with your preferred method
```

### Running
```bash
./3dObjViewer
```

## Controls
- WASD: Move camera
- QE: Move up/down
- Arrow keys: Rotate camera
- ESC: Exit

## Project Structure
See PROJECT_OVERVIEW.md for detailed information about the architecture and components.
