cmake_minimum_required(VERSION 3.16)
project(3dObjViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

# Include directories for external libraries
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

# Include directories for project headers
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/vulkan)
include_directories(${CMAKE_SOURCE_DIR}/src/rendering)
include_directories(${CMAKE_SOURCE_DIR}/src/geometry)
include_directories(${CMAKE_SOURCE_DIR}/src/math)
include_directories(${CMAKE_SOURCE_DIR}/src/utils)

# Set source files
set(SOURCES
    src/main.cpp
    src/Application.cpp
    
    # Vulkan engine
    src/vulkan/VulkanEngine.cpp
    src/vulkan/VulkanDevice.cpp
    src/vulkan/VulkanSwapchain.cpp
    src/vulkan/VulkanBuffer.cpp
    src/vulkan/VulkanImage.cpp
    src/vulkan/VulkanPipeline.cpp
    src/vulkan/VulkanUtils.cpp
    
    # Rendering
    src/rendering/Renderer.cpp
    src/rendering/Camera.cpp
    src/rendering/Light.cpp
    src/rendering/Mesh.cpp
    src/rendering/DynamicUBO.cpp
    
    # Geometry
    src/geometry/Model.cpp
    src/geometry/ObjLoader.cpp
    src/geometry/GeometryGenerator.cpp
    
    # Math
    src/math/Vector3.cpp
    src/math/Matrix4.cpp
    src/math/Transform.cpp
    
    # Utils
    src/utils/FileUtils.cpp
    src/utils/Logger.cpp
)

# Set header files
set(HEADERS
    src/Application.h
    
    # Vulkan engine
    src/vulkan/VulkanEngine.h
    src/vulkan/VulkanDevice.h
    src/vulkan/VulkanSwapchain.h
    src/vulkan/VulkanBuffer.h
    src/vulkan/VulkanImage.h
    src/vulkan/VulkanPipeline.h
    src/vulkan/VulkanUtils.h
    
    # Rendering
    src/rendering/Renderer.h
    src/rendering/Camera.h
    src/rendering/Light.h
    src/rendering/Mesh.h
    src/rendering/DynamicUBO.h
    
    # Geometry
    src/geometry/Model.h
    src/geometry/ObjLoader.h
    src/geometry/GeometryGenerator.h
    
    # Math
    src/math/Vector3.h
    src/math/Matrix4.h
    src/math/Transform.h
    
    # Utils
    src/utils/FileUtils.h
    src/utils/Logger.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${Vulkan_LIBRARIES}
    ${SDL2_LIBRARIES}
)

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::SDL2main)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
    if(APPLE)
        target_link_libraries(${PROJECT_NAME} SDL2)
    else()
        target_link_libraries(${PROJECT_NAME} SDL2)
    endif()
endif()

# Debug configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _DEBUG)
endif()

# Shader compilation setup
find_program(GLSL_VALIDATOR glslangValidator HINTS 
    ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
    /usr/bin 
    /usr/local/bin 
    /opt/homebrew/bin
    ${VULKAN_SDK_PATH}/Bin
    ${VULKAN_SDK_PATH}/Bin32
    $ENV{VULKAN_SDK}/Bin/ 
    $ENV{VULKAN_SDK}/Bin32/
    $ENV{VULKAN_SDK}/bin
    $ENV{VULKAN_SDK}/macOS/bin
)

# Custom target for shader compilation
if(GLSL_VALIDATOR)
    message(STATUS "Found glslangValidator: ${GLSL_VALIDATOR}")
    
    # Define shader source files (with proper extensions for glslangValidator)
    set(GLSL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/shaders/vertex.vert.glsl"
        "${CMAKE_SOURCE_DIR}/shaders/fragment.frag.glsl"
    )
    
    # Create output directory for compiled shaders
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
    
    # Generate SPIR-V files for each shader
    set(SPIRV_BINARY_FILES)
    foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.spv")
        
        # Add custom command to compile each shader
        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_VALIDATOR} -V --target-env vulkan1.0 ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
            COMMENT "Compiling shader: ${FILE_NAME} -> ${FILE_NAME}.spv"
            VERBATIM
        )
        
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(GLSL)
    
    # Create custom target that depends on all compiled shaders
    add_custom_target(
        CompileShaders 
        DEPENDS ${SPIRV_BINARY_FILES}
        COMMENT "Compiling all shaders"
    )
    
    # Make the main executable depend on compiled shaders
    add_dependencies(${PROJECT_NAME} CompileShaders)
    
    # Clean up compiled shaders on 'make clean'
    set_property(TARGET CompileShaders PROPERTY ADDITIONAL_CLEAN_FILES ${SPIRV_BINARY_FILES})
    
else()
    message(WARNING "glslangValidator not found. Shaders will not be compiled automatically.")
    message(STATUS "You can install it via: brew install glslang")
endif()

# Copy assets to build directory (this remains the same)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Copy shader source files to build directory for reference
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# Set the startup project for Visual Studio
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()