#include "LoadedModel.h"
#include "../geometry/Model.h"
#include "../geometry/ObjLoader.h"
#include "../rendering/Texture.h"
#include "../vulkan/VulkanEngine.h"
#include "../vulkan/VulkanDevice.h"
#include "../utils/JsonSupport.h"
#include <iostream>

LoadedModel::LoadedModel(const std::string& filepath, const std::string& name)
    : SceneObject(name)
    , filePath(filepath)
    , flipTextureY(false)  // Use original coordinate system.
    , cachedMesh(nullptr)
{ }

std::unique_ptr<Model> LoadedModel::createModel() const {
    auto model = std::make_unique<Model>();

    if (filePath.empty()) {
        std::cerr << "LoadedModel: No file path specified for " << name << std::endl;
        return model;
    }

    try {
        std::shared_ptr<Mesh> mesh = cachedMesh;

        if (!mesh) {	// Load mesh if not cached:
            ObjLoader loader(flipTextureY);
            auto result = loader.loadWithMaterial(filePath);
            mesh = result.mesh;
            cachedMesh = mesh; // Cache for future use.

            if (result.material.hasTexture()) {	// Store the material texture path for later loading.
                materialTexturePath = result.material.diffuseTexture;
            }
        }

        model->setMesh(mesh);
        model->setPosition(position);
        model->setRotation(rotation);
        model->setScale(scale);

        if (texture) {	// Apply texture if available.
            model->setTexture(texture);
        }

    } catch (const std::exception& e) {
        std::cerr << "LoadedModel: Failed to load " << filePath << ": " << e.what() << std::endl;
    }
    return model;
}

void LoadedModel::initializeTexture(VulkanDevice& device, VulkanEngine& engine) {
    // Force reload of material information if needed.
    if (materialTexturePath.empty() && !filePath.empty()) {
        try {
            ObjLoader loader(flipTextureY);
            auto result = loader.loadWithMaterial(filePath);
            if (result.material.hasTexture()) {
                materialTexturePath = result.material.diffuseTexture;
            }
        } catch (const std::exception& e) {
            std::cerr << "LoadedModel: Failed to reload material for " << name << ": " << e.what() << std::endl;
        }
    }

    if (texture)  // Don't reload if texture is already set.
        return;

    std::string texturePathToLoad;

    // Priority: explicit texturePath > material texture > none
    if (!texturePath.empty()) {
        texturePathToLoad = texturePath;
    } else if (!materialTexturePath.empty()) {
        // Handle material texture path - could be relative or absolute.
        if (materialTexturePath.find("assets/") == 0) {
            // Path already includes assets/, use as-is:
            texturePathToLoad = materialTexturePath;
        } else {
            // Relative path, resolve relative to model directory:
            std::string modelDir = filePath.substr(0, filePath.find_last_of("/\\"));
            texturePathToLoad = modelDir + "/" + materialTexturePath;
        }
    }

    if (!texturePathToLoad.empty()) {
        try {
            auto loadedTexture = std::make_shared<Texture>();
            if (loadedTexture->loadFromFile(texturePathToLoad, device, engine)) {
                texture = loadedTexture;
                std::cout << "Loaded texture for " << name << ": " << texturePathToLoad << std::endl;
            } else {
                std::cerr << "Failed to load texture for " << name << ": " << texturePathToLoad << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception loading texture for " << name << ": " << e.what() << std::endl;
        }
    }
}

json LoadedModel::serialize() const {
    json jsonData;
    jsonData["name"] = name;			// First
    jsonData["type"] = "LoadedModel";	// Second
    jsonData["filePath"] = filePath;	// Third
    jsonData["materialPath"] = materialPath;
    jsonData["texturePath"] = texturePath;
    jsonData["flipTextureY"] = flipTextureY;

    // Now merge the base class data...
    json baseData = SceneObject::serialize();
    jsonData["position"] = baseData["position"];
    jsonData["rotation"] = baseData["rotation"];
    jsonData["scale"] = baseData["scale"];

    return jsonData;
}

void LoadedModel::deserialize(const json& jsonData) {
    SceneObject::deserialize(jsonData);
    if (jsonData.contains("filePath")) {
        filePath = jsonData["filePath"].get<std::string>();
    }
    if (jsonData.contains("materialPath")) {
        materialPath = jsonData["materialPath"].get<std::string>();
    }
    if (jsonData.contains("texturePath")) {
        texturePath = jsonData["texturePath"].get<std::string>();
    }
    if (jsonData.contains("flipTextureY")) {
        flipTextureY = jsonData["flipTextureY"].get<bool>();
    }
}

std::unique_ptr<SceneObject> LoadedModel::clone() const {
    auto clone = std::make_unique<LoadedModel>(filePath, name + "_clone");
    copyBaseTo(clone.get());
    clone->materialPath = materialPath;
    clone->texturePath = texturePath;
    clone->flipTextureY = flipTextureY;
    // Don't copy cache - let clone load independently.
    return clone;
}
