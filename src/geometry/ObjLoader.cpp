#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <unordered_map>
#include <filesystem>

ObjLoader::ObjLoader(bool flipTextureY) : flipTextureY(flipTextureY) {
}

ObjLoader::~ObjLoader() {
}

std::shared_ptr<Mesh> ObjLoader::load(const std::string& filename) {
	std::string content = loadFile(filename);
	ObjData objData = parseObj(content);
	return buildMeshFromObjData(objData);
}

ObjLoader::ObjResult ObjLoader::loadWithMaterial(const std::string& filename) {
	std::string content = loadFile(filename);
	ObjData objData = parseObj(content);

	ObjResult result;
	result.mesh = buildMeshFromObjData(objData);

	// Load material if specified:
	if (!objData.materialLibrary.empty()) {
		try {
			std::string objDir = getDirectoryPath(filename);
			std::string mtlPath = objDir + "/" + objData.materialLibrary;
			auto materials = parseMtl(mtlPath);

			if (!objData.currentMaterial.empty() && materials.find(objData.currentMaterial) != materials.end()) {
				result.material = materials[objData.currentMaterial];
				std::cout << "Using material: " << result.material.name;
				if (result.material.hasTexture()) {
					std::cout << " with texture: " << result.material.diffuseTexture;
				}
				std::cout << std::endl;
			} else {
				std::cout << "Material not found or not specified, using default" << std::endl;
			}
		} catch (const std::exception& e) {
			std::cout << "Failed to load material: " << e.what() << std::endl;
		}
	}
	return result;
}

ObjLoader::ObjData ObjLoader::parseObj(const std::string& content) {
	ObjData data;
	std::istringstream stream(content);
	std::string line;

	while (std::getline(stream, line)) {
		if (line.empty() || line[0] == '#') {
			continue;
		}

		std::istringstream lineStream(line);
		std::string token;
		lineStream >> token;

		if (token == "v") {				// Vertex position
			data.positions.push_back(parseVector3(line));
		}
		else if (token == "vt") {		// Texture coordinate
			data.texCoords.push_back(parseVector2(line));
		}
		else if (token == "vn") {		// Vertex Normal
			data.normals.push_back(parseVector3(line));
		}
		else if (token == "f") {		// Face
			parseFace(line, data);
		}
		else if (token == "mtllib") {	// Material Library
			std::istringstream lineStream(line);
			std::string mtllib;
			lineStream >> token >> mtllib;  // Skip "mtllib" and get filename.
			data.materialLibrary = mtllib;
		}
		else if (token == "usemtl") {	// Use Material
			std::istringstream lineStream(line);
			std::string usemtl;
			lineStream >> token >> usemtl;  // Skip "usemtl" and get material name.
			data.currentMaterial = usemtl;
		}
	}

	// Generate normals if none were provided:
	if (data.normals.empty() && !data.positions.empty()) {
		generateNormals(data);
	}
	return data;
}

Vector3 ObjLoader::parseVector3(const std::string& line) {
	std::istringstream stream(line);
	std::string token;
	float x, y, z;

	stream >> token >> x >> y >> z;
	return Vector3(x, y, z);
}

Vector2 ObjLoader::parseVector2(const std::string& line) {
	std::istringstream stream(line);
	std::string token;
	float u, v;

	stream >> token >> u >> v;

	// Conditionally flip Y coordinate based on target graphics API.
	// Vulkan uses Y-down, OpenGL uses Y-up coordinate system.
	if (flipTextureY) {
		return Vector2(u, 1.0f - v);  // Flip for Vulkan.
	} else {
		return Vector2(u, v);         // Keep original for OpenGL.
	}
}

void ObjLoader::parseFace(const std::string& line, ObjData& data) {
	std::istringstream stream(line);
	std::string token;
	stream >> token; // Skip "f"

	std::vector<FaceVertex> faceVertices;

	while (stream >> token) {
		FaceVertex fv;
		fv.positionIndex = -1;
		fv.texCoordIndex = -1;
		fv.normalIndex = -1;

		// Parse vertex reference: v, v/vt, v/vt/vn, v//vn
		size_t slash1 = token.find('/');
		if (slash1 == std::string::npos) {
			// Format: v
			fv.positionIndex = std::stoi(token) - 1;  // Convert to 0-based
		} else {
			// Format with slashes:
			fv.positionIndex = std::stoi(token.substr(0, slash1)) - 1;

			size_t slash2 = token.find('/', slash1 + 1);
			if (slash2 != std::string::npos) {
				// Format: v/vt/vn or v//vn
				if (slash2 > slash1 + 1) {
					// Has texture coordinate
					fv.texCoordIndex = std::stoi(token.substr(slash1 + 1, slash2 - slash1 - 1)) - 1;
				}
				fv.normalIndex = std::stoi(token.substr(slash2 + 1)) - 1;
			} else {
				// Format: v/vt
				fv.texCoordIndex = std::stoi(token.substr(slash1 + 1)) - 1;
			}
		}

		faceVertices.push_back(fv);
	}

	// Triangulate face with counter-clockwise winding.
	// Reverse triangle order to ensure counter-clockwise winding.
	for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
		// Triangle fan triangulation with reversed order for CCW:
		data.faceVertices.push_back(faceVertices[0]);
		data.faceVertices.push_back(faceVertices[i + 1]);
		data.faceVertices.push_back(faceVertices[i]);

		// Also store simple indices if no separate normal indices:
		if (faceVertices[0].normalIndex == -1) {
			data.indices.push_back(faceVertices[0].positionIndex);
			data.indices.push_back(faceVertices[i + 1].positionIndex);
			data.indices.push_back(faceVertices[i].positionIndex);
		}
	}
}

void ObjLoader::generateNormals(ObjData& data) {
	data.generatedNormals.resize(data.positions.size(), Vector3::zero());

	// Lambda to calculate and accumulate normal for a triangle:
	auto accumulateTriangleNormal = [&data](uint32_t i0, uint32_t i1, uint32_t i2) {
		if (i0 < data.positions.size() && i1 < data.positions.size() && i2 < data.positions.size()) {
			Vector3 v0 = data.positions[i0];
			Vector3 v1 = data.positions[i1];
			Vector3 v2 = data.positions[i2];

			Vector3 edge1 = v1 - v0;
			Vector3 edge2 = v2 - v0;
			Vector3 normal = edge1.cross(edge2).normalized();

			data.generatedNormals[i0] += normal;
			data.generatedNormals[i1] += normal;
			data.generatedNormals[i2] += normal;
		}
	};

	// Calculate face normals and accumulate at vertices.
	if (!data.indices.empty()) {	// Use simple indices:
		for (size_t i = 0; i < data.indices.size(); i += 3) {
			accumulateTriangleNormal(data.indices[i], data.indices[i + 1], data.indices[i + 2]);
		}
	} else if (!data.faceVertices.empty()) {	// Use face vertices:
		for (size_t i = 0; i < data.faceVertices.size(); i += 3) {
			accumulateTriangleNormal(data.faceVertices[i].positionIndex,
									data.faceVertices[i + 1].positionIndex,
									data.faceVertices[i + 2].positionIndex);
		}
	}

	// Normalize accumulated normals:
	for (auto& normal : data.generatedNormals) {
		if (normal.length() > 0.0001f) {
			normal.normalize();
		} else {
			normal = Vector3(0.0f, 1.0f, 0.0f);  // Default up normal.
		}
	}
}

std::unordered_map<std::string, ObjLoader::Material> ObjLoader::parseMtl(const std::string& filename) {
	std::unordered_map<std::string, Material> materials;
	std::string content = loadFile(filename);
	std::istringstream stream(content);
	std::string line;

	Material* currentMaterial = nullptr;

	while (std::getline(stream, line)) {
		if (line.empty() || line[0] == '#') {
			continue;
		}

		std::istringstream lineStream(line);
		std::string token;
		lineStream >> token;

		if (token == "newmtl") {		// New Material
			std::string materialName;
			lineStream >> materialName;
			materials[materialName] = Material();
			materials[materialName].name = materialName;
			currentMaterial = &materials[materialName];
		}
		else if (currentMaterial) {
			if (token == "map_Kd") {	// Diffuse texture map
				std::string texturePath;
				lineStream >> texturePath;

				// Resolve relative texture path:
				std::string mtlDir = getDirectoryPath(filename);
				if (!mtlDir.empty()) {
					currentMaterial->diffuseTexture = mtlDir + "/" + texturePath;
				} else {
					currentMaterial->diffuseTexture = texturePath;
				}
			}
			else if (token == "Kd") {	// Diffuse color
				float r, g, b;
				lineStream >> r >> g >> b;
				currentMaterial->diffuseColor = Vector3(r, g, b);
			}
			else if (token == "Ka") {	// Ambient color
				float r, g, b;
				lineStream >> r >> g >> b;
				currentMaterial->ambientColor = Vector3(r, g, b);
			}
			else if (token == "Ks") {	// Specular color
				float r, g, b;
				lineStream >> r >> g >> b;
				currentMaterial->specularColor = Vector3(r, g, b);
			}
			else if (token == "Ns") {	// Shininess
				float shininess;
				lineStream >> shininess;
				currentMaterial->shininess = shininess;
			}
		}
	}
	std::cout << "Loaded " << materials.size() << " materials from " << filename << std::endl;
	return materials;
}

std::string ObjLoader::getDirectoryPath(const std::string& filepath) {
	size_t lastSlash = filepath.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		return filepath.substr(0, lastSlash);
	}
	return ""; // No directory path.
}

std::string ObjLoader::loadFile(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::ostringstream content;
	content << file.rdbuf();
	return content.str();
}

std::shared_ptr<Mesh> ObjLoader::buildMeshFromObjData(const ObjData& objData) {
	auto mesh = std::make_shared<Mesh>();

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	bool hasTextureCoords = false;

	if (!objData.faceVertices.empty()) {
		processFaceVertices(objData, vertices, indices, hasTextureCoords);
	} else {
		processIndexedVertices(objData, vertices, indices, hasTextureCoords);
	}

	std::cout << "Loaded OBJ with " << vertices.size() << " vertices and "
			  << indices.size() / 3 << " triangles";
	if (hasTextureCoords) {
		std::cout << " (textured)";
	}
	std::cout << std::endl;

	mesh->setVertices(vertices);
	mesh->setIndices(indices);
	mesh->setHasTexture(hasTextureCoords);

	return mesh;
}

void ObjLoader::processFaceVertices(const ObjData& objData, std::vector<Vertex>& vertices,
									std::vector<uint32_t>& indices, bool& hasTextureCoords) {
	std::unordered_map<std::string, uint32_t> uniqueVertices;

	for (const auto& faceVert : objData.faceVertices) {
		std::string key = std::to_string(faceVert.positionIndex) + "/" +
						 std::to_string(faceVert.texCoordIndex) + "/" +
						 std::to_string(faceVert.normalIndex);

		auto it = uniqueVertices.find(key);
		if (it == uniqueVertices.end()) {
			Vertex vertex = createVertex(objData, faceVert, hasTextureCoords);
			uint32_t newIndex = static_cast<uint32_t>(vertices.size());
			vertices.push_back(vertex);
			uniqueVertices[key] = newIndex;
			indices.push_back(newIndex);
		} else {
			indices.push_back(it->second);
		}
	}
}

void ObjLoader::processIndexedVertices(const ObjData& objData, std::vector<Vertex>& vertices,
									   std::vector<uint32_t>& indices, bool& hasTextureCoords) {
	vertices.reserve(objData.positions.size());
	hasTextureCoords = !objData.texCoords.empty();

	for (size_t i = 0; i < objData.positions.size(); ++i) {
		vertices.push_back(createVertex(objData, i, hasTextureCoords));
	}
	indices = objData.indices;
}

Vertex ObjLoader::createVertex(const ObjData& objData, const FaceVertex& faceVert, bool& hasTextureCoords) {
	Vertex vertex;
																				// GET:
	if (faceVert.positionIndex >= 0 && faceVert.positionIndex < (int) objData.positions.size()) {
		vertex.position = objData.positions[faceVert.positionIndex];			//	Position
	}

	if (faceVert.texCoordIndex >= 0 && faceVert.texCoordIndex < (int) objData.texCoords.size()) {
		vertex.texCoord = objData.texCoords[faceVert.texCoordIndex];			//	Texture Coordinates
		hasTextureCoords = true;
	} else {
		vertex.texCoord = Vector2(0.0f, 0.0f);									//		"
	}
	vertex.normal = determineVertexNormal(objData, faceVert.normalIndex,		//	Normal
										  faceVert.positionIndex);
	vertex.color = determineVertexColor(vertex.position, hasTextureCoords);		//	Color

	return vertex;
}

Vertex ObjLoader::createVertex(const ObjData& objData, size_t index, bool hasTextureCoords) {
	Vertex vertex;
																				// GET:
	vertex.position = objData.positions[index];									//	Position

	if (index < objData.texCoords.size()) {										//	Texture Coordinates
		vertex.texCoord = objData.texCoords[index];
	} else {
		vertex.texCoord = Vector2(0.0f, 0.0f);									//		"
	}
	int normalIndex = (index < objData.normals.size()) ? index : -1;

	vertex.normal = determineVertexNormal(objData, normalIndex, index);			//	Normal

	vertex.color = determineVertexColor(vertex.position, hasTextureCoords);		//	Color

	return vertex;
}

Vector3 ObjLoader::determineVertexNormal(const ObjData& objData, int normalIndex, int positionIndex) {
	if (normalIndex >= 0 && normalIndex < (int) objData.normals.size()) {
		return objData.normals[normalIndex];
	} else if (!objData.generatedNormals.empty() && positionIndex >= 0 &&
			   positionIndex < (int) objData.generatedNormals.size()) {
		return objData.generatedNormals[positionIndex];
	} else {
		return Vector3(0.0f, 1.0f, 0.0f);  // Default UP normal.
	}
}

Vector3 ObjLoader::determineVertexColor(const Vector3& position, bool hasTextureCoords) {
	if (hasTextureCoords) {
		return Vector3(1.0f, 1.0f, 1.0f);	// White for textured models, otherwise
	} else {
		return Vector3(						//	generate color based on position.
			(position.x + 1.0f) * 0.5f,
			(position.y + 1.0f) * 0.5f,
			(position.z + 1.0f) * 0.5f
		);
	}
}
