//
// SceneObject.cpp
//
// Abstract object present in level.
//
#include "SceneObject.h"
#include "../geometry/Model.h"
#include "../utils/JsonSupport.h"

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
    // Don't set name here - let derived classes set it first

    // Serialize position
    jsonData["position"] = json::object({
        {"x", position.x},
        {"y", position.y},
        {"z", position.z}
    });

    // Serialize rotation
    jsonData["rotation"] = json::object({
        {"x", rotation.x},
        {"y", rotation.y},
        {"z", rotation.z}
    });

    // Serialize scale
    jsonData["scale"] = json::object({
        {"x", scale.x},
        {"y", scale.y},
        {"z", scale.z}
    });

    return jsonData;
}

void SceneObject::deserialize(const json& jsonData) {
    if (jsonData.contains("name")) {
        name = jsonData["name"].get<std::string>();
    }

    if (jsonData.contains("position")) {
        const json& posJson = jsonData["position"];
        if (posJson.contains("x")) position.x = posJson["x"].get<float>();
        if (posJson.contains("y")) position.y = posJson["y"].get<float>();
        if (posJson.contains("z")) position.z = posJson["z"].get<float>();
    }

    if (jsonData.contains("rotation")) {
        const json& rotJson = jsonData["rotation"];
        if (rotJson.contains("x")) rotation.x = rotJson["x"].get<float>();
        if (rotJson.contains("y")) rotation.y = rotJson["y"].get<float>();
        if (rotJson.contains("z")) rotation.z = rotJson["z"].get<float>();
    }

    if (jsonData.contains("scale")) {
        const json& scaleJson = jsonData["scale"];
        if (scaleJson.contains("x")) scale.x = scaleJson["x"].get<float>();
        if (scaleJson.contains("y")) scale.y = scaleJson["y"].get<float>();
        if (scaleJson.contains("z")) scale.z = scaleJson["z"].get<float>();
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
