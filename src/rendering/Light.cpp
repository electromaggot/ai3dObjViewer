#include "Light.h"

Light::Light(LightType type)
    : type(type)
    , position(0.0f, 0.0f, 0.0f)
    , direction(0.0f, -1.0f, 0.0f)
    , color(1.0f, 1.0f, 1.0f)
    , intensity(1.0f)
    , constantAttenuation(1.0f)
    , linearAttenuation(0.09f)
    , quadraticAttenuation(0.032f)
    , innerSpotAngle(12.5f)
    , outerSpotAngle(17.5f)
    , enabled(true)
{
}

Light::~Light() {
}

void Light::setPosition(const Vector3& position) {
    this->position = position;
}

void Light::setDirection(const Vector3& direction) {
    this->direction = direction.normalized();
}

void Light::setColor(const Vector3& color) {
    this->color = color;
}

void Light::setIntensity(float intensity) {
    this->intensity = intensity;
}

void Light::setAttenuation(float constant, float linear, float quadratic) {
    this->constantAttenuation = constant;
    this->linearAttenuation = linear;
    this->quadraticAttenuation = quadratic;
}

void Light::setSpotAngle(float innerAngle, float outerAngle) {
    this->innerSpotAngle = innerAngle;
    this->outerSpotAngle = outerAngle;
}