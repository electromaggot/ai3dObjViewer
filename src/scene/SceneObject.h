#pragma once

#include "../math/Vector3.h"
#include "../math/Matrix4.h"
#include "../rendering/Mesh.h"
#include "../rendering/Texture.h"
#include <memory>
#include <string>
#include "JsonSupport.h"

// Forward declarations
class Model;
class VulkanDevice;
class VulkanEngine;


/**
 * Abstract base class for all objects in the 3D scene.
 * Follows Liskov Substitution Principle - any derived class can be used
 * wherever a SceneObject is expected.
 */
class SceneObject {
public:
    enum class ObjectType {
        PROCEDURAL_MODEL,   // Generated geometry (cube, sphere, etc.)
        LOADED_MODEL,       // Models loaded from files (OBJ, FBX, etc.)
        LIGHT_SOURCE,       // Future extension
        PARTICLE_EMITTER,   // Future extension
        CAMERA,             // Future extension for multiple cameras
        CUSTOM              // User-defined types
    };

    SceneObject(const std::string& name = "Unnamed Object");
    virtual ~SceneObject() = default;

    // Core interface that all scene objects must implement
    virtual ObjectType getType() const = 0;
    virtual std::unique_ptr<Model> createModel() const = 0;
    virtual json serialize() const;
    virtual void deserialize(const json& jsonData);

    // Common properties for all scene objects
    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }

    const Vector3& getPosition() const { return position; }
    void setPosition(const Vector3& pos) { position = pos; }

    const Vector3& getRotation() const { return rotation; }
    void setRotation(const Vector3& rot) { rotation = rot; }

    const Vector3& getScale() const { return scale; }
    void setScale(const Vector3& s) { scale = s; }

    bool isVisible() const { return visible; }
    void setVisible(bool v) { visible = v; }

    bool hasTexture() const { return texture != nullptr; }
    std::shared_ptr<Texture> getTexture() const { return texture; }
    void setTexture(std::shared_ptr<Texture> tex) { texture = tex; }

    // Transformation matrix
    Matrix4 getTransformMatrix() const;

    // Clone method for prototype pattern (useful for scene editing)
    virtual std::unique_ptr<SceneObject> clone() const = 0;

protected:
    std::string name;
    Vector3 position;
    Vector3 rotation;  // Euler angles in degrees
    Vector3 scale;
    bool visible;
    std::shared_ptr<Texture> texture;

    // Helper method for derived classes
    void copyBaseTo(SceneObject* other) const;
};
