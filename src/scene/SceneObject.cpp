//
// SceneObject.cpp
//
// Abstract object present in level.
//
#include "SceneObject.h"
#include "../geometry/Model.h"
#include "JsonSupport.h"

SceneObject::SceneObject(const std::string& name)
    : name(name)
    , position(0.0f, 0.0f, 0.0f)
    , rotation(0.0f, 0.0f, 0.0f)
    , scale(1.0f, 1.0f, 1.0f)
    , visible(true)
    , texture(nullptr)
{ }

Matrix4 SceneObject::getTransformMatrix() const {
    // Build transformation matrix: scale * rotation * translation
    Matrix4 scaleMatrix = Matrix4::scale(scale);
    Matrix4 rotationMatrix = Matrix4::rotation(rotation);
    Matrix4 translationMatrix = Matrix4::translation(position);

    return translationMatrix * rotationMatrix * scaleMatrix;
}

json SceneObject::serialize() const {
    json jsonData;
    jsonData["name"] = name;
    // Note: Actual implementation would populate with position, rotation, scale, etc.
    // For now, this is a stub that provides the interface.
    return jsonData;
}

void SceneObject::deserialize(const json& jsonData) {
    // Note: Actual implementation would read position, rotation, scale, etc.
    // For now, this is a stub that provides the interface.
    if (jsonData.contains("name")) {
        name = static_cast<std::string>(jsonData["name"]);
    }
}

void SceneObject::copyBaseTo(SceneObject* other) const {
    other->name = name;
    other->position = position;
    other->rotation = rotation;
    other->scale = scale;
    other->visible = visible;
    other->texture = texture; // Shallow copy - textures are shared.
}
