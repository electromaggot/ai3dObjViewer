#include "GeneratedModel.h"
#include "../geometry/Model.h"
#include "../geometry/GeometryGenerator.h"
#include "JsonSupport.h"


GeneratedModel::GeneratedModel(Shape shape, const std::string& name)
    : SceneObject(name)
    , shape(shape)
{
    initializeDefaults();
}

void GeneratedModel::initializeDefaults() {
    // Set sensible defaults based on shape type
    switch (shape) {
        case Shape::CUBE:
            param1 = 1.0f;  // size
            param2 = 0.0f;  // unused
            segments = 1;   // unused
            break;
        case Shape::SPHERE:
            param1 = 1.0f;  // radius
            param2 = 0.0f;  // unused
            segments = 24;  // tessellation
            break;
        case Shape::CYLINDER:
            param1 = 0.5f;  // radius
            param2 = 1.0f;  // height
            segments = 20;  // tessellation
            break;
        case Shape::PLANE:
            param1 = 1.0f;  // width
            param2 = 1.0f;  // height
            segments = 1;   // unused
            break;
        case Shape::DODECAHEDRON:
            param1 = 1.0f;  // size
            param2 = 0.0f;  // unused
            segments = 1;   // unused
            break;
        default:
            param1 = 1.0f;
            param2 = 1.0f;
            segments = 24;
            break;
    }
}

std::unique_ptr<Model> GeneratedModel::createModel() const {
    auto model = std::make_unique<Model>();

    // Generate the appropriate mesh based on shape
    std::shared_ptr<Mesh> mesh;
    switch (shape) {
        case Shape::CUBE:
            mesh = GeometryGenerator::createCube(param1);
            break;
        case Shape::SPHERE:
            mesh = GeometryGenerator::createSphere(param1, segments);
            break;
        case Shape::CYLINDER:
            mesh = GeometryGenerator::createCylinder(param1, param2, segments);
            break;
        case Shape::PLANE:
            mesh = GeometryGenerator::createPlane(param1, param2);
            break;
        case Shape::DODECAHEDRON:
            mesh = GeometryGenerator::createDodecahedron(param1);
            break;
        default:
            mesh = GeometryGenerator::createCube(1.0f); // fallback
            break;
    }

    model->setMesh(mesh);
    model->setPosition(position);
    model->setRotation(rotation);
    model->setScale(scale);

    if (texture) {
        model->setTexture(texture);
    }

    return model;
}

json GeneratedModel::serialize() const {
    json jsonData = SceneObject::serialize();
    jsonData["type"] = "GeneratedModel";
    jsonData["shape"] = name();
    // Note: Actual implementation would include param1, param2, segments
    return jsonData;
}

void GeneratedModel::deserialize(const json& jsonData) {
    SceneObject::deserialize(jsonData);
    // Note: Actual implementation would read shape, param1, param2, segments
    // For now, this is a stub that provides the interface
}

std::unique_ptr<SceneObject> GeneratedModel::clone() const {
    auto clone = std::make_unique<GeneratedModel>(shape, std::string(name()) + "_clone");
    copyBaseTo(clone.get());
    clone->param1 = param1;
    clone->param2 = param2;
    clone->segments = segments;
    return clone;
}
