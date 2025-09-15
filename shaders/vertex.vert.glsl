#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 lightPos;
    vec3 lightColor;
    vec3 viewPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 lightPos;
layout(location = 4) out vec3 lightColor;
layout(location = 5) out vec3 viewPos;

void main() {
    // Transform position to world space
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;
    
    // Transform position to clip space
    gl_Position = ubo.proj * ubo.view * worldPos;
    
    // Transform normal to world space
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
    
    // Pass through vertex color and lighting parameters
    fragColor = inColor;
    lightPos = ubo.lightPos;
    lightColor = ubo.lightColor;
    viewPos = ubo.viewPos;
}