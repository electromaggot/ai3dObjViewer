*This repository is a work-in-progress, but feel free to play around with it!  I'll do my best to keep it in a working state, so you can clone/pull and build/run the `ViewerProject`.  Currently supports Apple (macOS, iOS) and Linux (Ubuntu, Raspberry Pi 5).  Windows support is currently work-in-progress.  (This comment is of course temporary.)*

<img src="https://github.com/electromaggot/VulkanAddOns/blob/main/ViewerProject/Xcode/Resources/iOS/Images.xcassets/AppIcon-iOS.appiconset/ItunesArtwork%402x.png" width="100" height="100" />

# VulkanAddOns
Adds graphical elements to [VulkanModule](https://github.com/electromaggot/VulkanModule) which sets up Vulkan.  Some of what you'll find here:
- ViewerProject is the buildable test project &ndash; an object viewer, evolving into a simple scene editor.
- Graphics Objects
   - 3D test objects, flexible and reusable, but some also self-contained using entirely pre-initialized data.<br>
     For instance: test quads (sprite/billboard), test cubes, geometric primitives, shapes like Utah teapot, etc.
   - coordinated shaders and test assets.
   - 3D Models (loadable from OBJ files, firstly).
   - 3D Fonts (generated from TrueType/OpenType files) [coming soon].
- gxEngine &ndash; components of a game/graphics engine.
- Particle System [coming later].

## Setup
Download this repository including its dependent projects:
```
git clone --recurse-submodules https://github.com/electromaggot/VulkanAddOns
```

