#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 lightPos;
layout(location = 4) in vec3 lightColor;
layout(location = 5) in vec3 viewPos;

layout(location = 0) out vec4 outColor;

void main() {
	// Normalize the fragment normal
	vec3 norm = normalize(fragNormal);

	// Use vertex color directly (no texture sampling)
	vec3 baseColor = fragColor;

	// Ambient lighting
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse lighting
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// Specular lighting (Phong)
	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	// Combine all lighting components
	vec3 lighting = ambient + diffuse + specular;
	vec3 result = lighting * baseColor;

	outColor = vec4(result, 1.0);
}