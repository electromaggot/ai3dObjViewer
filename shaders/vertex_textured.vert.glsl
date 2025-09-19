#version 450
#extension GL_ARB_separate_shader_objects : enable

// Global uniforms (binding 0) - same for all objects
layout(binding = 0) uniform GlobalUniforms {
	mat4 view;
	mat4 proj;
	vec4 lightPos;
	vec4 lightColor;
	vec4 viewPos;
} global;

// Per-object uniforms (binding 1) - different for each object via dynamic offsets
layout(binding = 1) uniform PerObjectUniforms {
	mat4 model;
	mat4 normalMatrix;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out vec3 lightPos;
layout(location = 5) out vec3 lightColor;
layout(location = 6) out vec3 viewPos;

void main() {
	// Transform position to world space
	vec4 worldPos = object.model * vec4(inPosition, 1.0);
	fragPos = worldPos.xyz;

	// Transform position to clip space
	gl_Position = global.proj * global.view * worldPos;

	// Transform normal to world space (using precomputed normal matrix)
	fragNormal = mat3(object.normalMatrix) * inNormal;

	// Pass through vertex color, texture coordinates, and lighting parameters
	fragColor = inColor;
	fragTexCoord = inTexCoord;
	lightPos = global.lightPos.xyz;
	lightColor = global.lightColor.xyz;
	viewPos = global.viewPos.xyz;
}