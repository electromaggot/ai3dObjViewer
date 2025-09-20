#include "SceneManager.h"
#include "GeneratedModel.h"
#include "LoadedModel.h"
#include "../geometry/Model.h"
#include "../utils/JsonSupport.h"
#include "../utils/logger/Logging.h"
#include <fstream>
#include <algorithm>

void SceneManager::addObject(std::unique_ptr<SceneObject> object) {
	if (!object)
		return;

	// Ensure unique names:
	std::string originalName = object->getName();
	std::string uniqueName = makeUniqueName(originalName);
	object->setName(uniqueName);

	objects.push_back(std::move(object));
}

void SceneManager::removeObject(const std::string& name) {
	size_t index = findObjectIndex(name);
	if (index < objects.size()) {
		removeObject(index);
	}
}

void SceneManager::removeObject(size_t index) {
	if (index < objects.size()) {
		objects.erase(objects.begin() + index);
	}
}

void SceneManager::clear() {
	objects.clear();
}

SceneObject* SceneManager::findObject(const std::string& name) const {
	auto it = std::find_if(objects.begin(), objects.end(),
		[&name](const std::unique_ptr<SceneObject>& obj) {
			return obj->getName() == name;
		});
	return (it != objects.end()) ? it->get() : nullptr;
}

SceneObject* SceneManager::getObject(size_t index) const {
	return (index < objects.size()) ? objects[index].get() : nullptr;
}

std::vector<std::unique_ptr<Model>> SceneManager::createAllModels() const {
	std::vector<std::unique_ptr<Model>> models;
	models.reserve(objects.size());

	for (const auto& object : objects) {
		if (object && object->isVisible()) {
			auto model = object->createModel();
			if (model) {
				models.push_back(std::move(model));
			}
		}
	}
	return models;
}

std::unique_ptr<Model> SceneManager::createModelForObject(const std::string& name) const {
	SceneObject* object = findObject(name);
	return object ? object->createModel() : nullptr;
}

json SceneManager::serialize() const {
	json jsonData;
	jsonData["version"] = "1.0";
	jsonData["objectCount"] = static_cast<double>(objects.size());

	json objectArray;
	for (const auto& object : objects) {
		if (object) {
			objectArray.push_back(object->serialize());
		}
	}
	jsonData["objects"] = objectArray;

	return jsonData;
}

void SceneManager::deserialize(const json& jsonData) {
	clear();

	if (!jsonData.contains("objects") || !jsonData["objects"].is_array()) {
		return;
	}

	// Iterate through objects and recreate them based on type.
	const json& objectsArray = jsonData["objects"];
	for (size_t i = 0; i < objectsArray.size(); ++i) {
		const json& objData = objectsArray[i];

		if (!objData.contains("type")) {
			Log(ERROR, "SceneManager: Object missing type field, skipping");
			continue;
		}

		std::string typeStr = static_cast<std::string>(objData["type"]);

		if (typeStr == "GeneratedModel") {
			auto generatedModel = std::make_unique<GeneratedModel>(GeneratedModel::Shape::CUBE);
			generatedModel->deserialize(objData);
			addObject(std::move(generatedModel));
		} else if (typeStr == "LoadedModel") {
			auto loadedModel = std::make_unique<LoadedModel>("", "");
			loadedModel->deserialize(objData);
			addObject(std::move(loadedModel));
		} else {
			Log(ERROR, "SceneManager: Unknown object type: %s", typeStr.c_str());
		}
	}
}

bool SceneManager::saveToFile(const std::string& filename) const {
	try {
		json jsonData = serialize();
		std::ofstream file(filename);
		if (!file.is_open()) {
			Log(ERROR, "SceneManager: Failed to open file for writing: %s", filename.c_str());
			return false;
		}

		file << jsonData.dump(4); // Pretty print with 4-space indentation.
		Log(NOTE, "Scene saved to: %s", filename.c_str());
		return true;
	} catch (const std::exception& e) {
		Log(ERROR, "SceneManager: Failed to save scene: %s", e.what());
		return false;
	}
}

bool SceneManager::loadFromFile(const std::string& filename) {
	try {
		std::ifstream file(filename);
		if (!file.is_open()) {
			Log(ERROR, "SceneManager: Failed to open file for reading: %s", filename.c_str());
			return false;
		}

		json jsonData;

		#ifdef JSON_LITE
		// For lightweight implementation, read entire file and parse manually:
			std::string fileContent((std::istreambuf_iterator<char>(file)),
									 std::istreambuf_iterator<char>());
			jsonData = JsonValue::parse(fileContent);
		#else
			// For real nlohmann::json, use stream operator:
			file >> jsonData;
		#endif

		deserialize(jsonData);

		Log(NOTE, "Scene loaded from: %s (%zu objects)", filename.c_str(), objects.size());
		return true;
	} catch (const std::exception& e) {
		Log(ERROR, "SceneManager: Failed to load scene: %s", e.what());
		return false;
	}
}

std::vector<std::string> SceneManager::getObjectNames() const {
	std::vector<std::string> names;
	names.reserve(objects.size());

	for (const auto& object : objects) {
		if (object) {
			names.push_back(object->getName());
		}
	}
	return names;
}

size_t SceneManager::getObjectCountByType(SceneObject::ObjectType type) const {
	return std::count_if(objects.begin(), objects.end(),
		[type](const std::unique_ptr<SceneObject>& obj) {
			return obj && obj->getType() == type;
		});
}

void SceneManager::addGeneratedModel(GeneratedModel::Shape shape, const Vector3& position,
									 float param1, float param2, int segments) {
	auto model = std::make_unique<GeneratedModel>(shape);
	model->setPosition(position);
	model->setParameter1(param1);
	model->setParameter2(param2);
	model->setSegments(segments);
	addObject(std::move(model));
}

void SceneManager::addLoadedModel(const std::string& name, const Vector3& position, const std::string& filepath) {
	auto model = std::make_unique<LoadedModel>(filepath, name);
	model->setPosition(position);
	addObject(std::move(model));
}

std::string SceneManager::makeUniqueName(const std::string& baseName) const {
	std::string uniqueName = baseName;
	int counter = 1;

	while (findObject(uniqueName) != nullptr) {
		uniqueName = baseName + "_" + std::to_string(counter);
		counter++;
	}
	return uniqueName;
}

size_t SceneManager::findObjectIndex(const std::string& name) const {
	for (size_t i = 0; i < objects.size(); ++i) {
		if (objects[i] && objects[i]->getName() == name) {
			return i;
		}
	}
	return objects.size(); // Invalid index
}
