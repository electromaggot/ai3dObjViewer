#pragma once

#include "SceneObject.h"
#include <string>

/**
 * Scene object for models loaded from external files (OBJ, FBX, etc.)
 */
class LoadedModel : public SceneObject {
public:
	LoadedModel(const std::string& filepath = "", const std::string& name = "Loaded Model");
	virtual ~LoadedModel() = default;

	// SceneObject interface
	ObjectType getType() const override { return ObjectType::LOADED_MODEL; }
	std::unique_ptr<Model> createModel() const override;
	json serialize() const override;
	void deserialize(const json& jsonData) override;
	std::unique_ptr<SceneObject> clone() const override;

	// LoadedModel-specific methods
	const std::string& getFilePath() const { return filePath; }
	void setFilePath(const std::string& path) { filePath = path; }

	const std::string& getMaterialPath() const { return materialPath; }
	void setMaterialPath(const std::string& path) { materialPath = path; }

	const std::string& getTexturePath() const { return texturePath; }
	void setTexturePath(const std::string& path) { texturePath = path; }

	bool getFlipTextureY() const { return flipTextureY; }
	void setFlipTextureY(bool flip) { flipTextureY = flip; }

	// Texture initialization - should be called after VulkanDevice/Engine are available.
	void initializeTexture(class VulkanDevice& device, class VulkanEngine& engine);

	// Cache management
	bool isCached() const { return cachedMesh != nullptr; }
	void clearCache() { cachedMesh.reset(); }

private:
	std::string filePath;		// Path to the model file
	std::string materialPath;	// Path to material file (if separate)
	std::string texturePath;	// Override texture path (if not from material)
	bool flipTextureY;			// Whether to flip texture Y coordinate

	// Cached mesh and material data to avoid reloading.
	mutable std::shared_ptr<Mesh> cachedMesh;
	mutable std::string materialTexturePath;  // Texture path from material file
};
