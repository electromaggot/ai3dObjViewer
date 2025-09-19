#pragma once

#include "SceneObject.h"
#include "GeneratedModel.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "../utils/JsonSupport.h"

// Forward declarations
class Model;
class VulkanDevice;
class VulkanEngine;


/**
 * Manages all objects in the 3D scene.
 * Provides facilities for adding, removing, finding, and serializing scene objects.
 */
class SceneManager {
public:
	SceneManager() = default;
	~SceneManager() = default;

	// Scene object management
	void addObject(std::unique_ptr<SceneObject> object);
	void removeObject(const std::string& name);
	void removeObject(size_t index);
	void clear();

	// Access methods
	SceneObject* findObject(const std::string& name) const;
	SceneObject* getObject(size_t index) const;
	size_t getObjectCount() const { return objects.size(); }

	// Iteration support
	std::vector<std::unique_ptr<SceneObject>>::const_iterator begin() const { return objects.begin(); }
	std::vector<std::unique_ptr<SceneObject>>::const_iterator end() const { return objects.end(); }

	// Model creation for rendering
	std::vector<std::unique_ptr<Model>> createAllModels() const;
	std::unique_ptr<Model> createModelForObject(const std::string& name) const;

	// Scene serialization for save/load
	json serialize() const;
	void deserialize(const json& jsonData);
	bool saveToFile(const std::string& filename) const;
	bool loadFromFile(const std::string& filename);

	// Utility methods
	std::vector<std::string> getObjectNames() const;
	size_t getObjectCountByType(SceneObject::ObjectType type) const;

	// Factory methods for common objects
	void addGeneratedModel(GeneratedModel::Shape shape, const Vector3& position,
						   float param1 = 1.0f, float param2 = 1.0f, int segments = 24);
	void addLoadedModel(const std::string& name, const Vector3& position, const std::string& filepath);

private:
	std::vector<std::unique_ptr<SceneObject>> objects;

	// Helper methods
	std::string makeUniqueName(const std::string& baseName) const;
	size_t findObjectIndex(const std::string& name) const;
};
