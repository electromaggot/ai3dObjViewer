#pragma once

#include "math/Vector3.h"

enum class LightType {
	DIRECTIONAL,
	POINT,
	SPOT
};

class Light {
public:
	Light(LightType type = LightType::POINT);
	~Light();

	// Position and direction
	void setPosition(const Vector3& position);
	void setDirection(const Vector3& direction);
	Vector3 getPosition() const { return position; }
	Vector3 getDirection() const { return direction; }

	// Light properties
	void setColor(const Vector3& color);
	void setIntensity(float intensity);
	Vector3 getColor() const { return color * intensity; }
	float getIntensity() const { return intensity; }

	// Light type
	LightType getType() const { return type; }
	void setType(LightType type) { this->type = type; }

	// Attenuation (for point and spot lights)
	void setAttenuation(float constant, float linear, float quadratic);
	float getConstantAttenuation() const { return constantAttenuation; }
	float getLinearAttenuation() const { return linearAttenuation; }
	float getQuadraticAttenuation() const { return quadraticAttenuation; }

	// Spot light properties
	void setSpotAngle(float innerAngle, float outerAngle);
	float getInnerSpotAngle() const { return innerSpotAngle; }
	float getOuterSpotAngle() const { return outerSpotAngle; }

	// Enable/disable
	void setEnabled(bool enabled) { this->enabled = enabled; }
	bool isEnabled() const { return enabled; }

private:
	LightType type;
	Vector3 position;
	Vector3 direction;
	Vector3 color;
	float intensity;

	// Attenuation parameters
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;

	// Spot light parameters
	float innerSpotAngle;
	float outerSpotAngle;

	bool enabled;
};